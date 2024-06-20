//
//  main.m
//  edf
//
//  Created by Manuel Cabrerizo on 18/06/2024.
//

// game includes
#include "edf_platform.h"
#include "edf_memory.h"
#include "edf.h"
// ios includes
#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import<MetalKit/MetalKit.h>
#include "ios_shader_types.h"

#define MAX_FRAMES_IN_FLIGHT 3
#define MAX_UBUFFER_COUNT 3
#define MAX_QUAD_COUNT 10000

static const Vertex quad_vertices[] = {
    { {  0.5,  -0.5 },  { 1.f, 0.f, 0.f }, {1, 1} },
    { { -0.5,  -0.5 },  { 0.f, 1.f, 0.f }, {0, 1} },
    { { -0.5,   0.5 },  { 0.f, 0.f, 1.f }, {0, 0} },

    { {  0.5,  -0.5 },  { 1.f, 0.f, 0.f }, {1, 1} },
    { { -0.5,   0.5 },  { 0.f, 0.f, 1.f }, {0, 0} },
    { {  0.5,   0.5 },  { 1.f, 0.f, 1.f }, {1, 0} }
};

typedef struct IosRenderer {
    MTKView *view;
    id<MTLDevice> device;
    id<MTLCommandQueue> command_queue;

    id<MTLCommandBuffer> command_buffer;
    id<MTLRenderCommandEncoder> command_encoder;

    dispatch_semaphore_t in_flight_semaphore;
    u32 current_buffer;

    id<MTLRenderPipelineState> alpha_blend_state;    
    id<MTLRenderPipelineState> additive_blend_state;    
    id<MTLBuffer> vbuffer;
    id<MTLBuffer> ubuffer[MAX_FRAMES_IN_FLIGHT][MAX_UBUFFER_COUNT];
    u32 quad_count;
    u32 buffer_index;

    matrix_float4x4 proj_m4;
    matrix_float4x4 view_m4;
} IosRenderer;


// globals variables
static Memory g_memory;

// temporal globals (after the gpu_load this variables are set to nil)
static MTKView *tmp_view;



//=====================================================================
// IOS utility functions
//=====================================================================
matrix_float4x4 matrix4x4_scale(float sx, float sy, float sz) {
    return (matrix_float4x4) {{
        { sx,  0,  0,  0 },
        {  0, sy,  0,  0 },
        {  0,  0, sz,  0 },
        {  0,  0,  0,  1 }
    }};
}

matrix_float4x4 matrix4x4_translation(float tx, float ty, float tz) {
    return (matrix_float4x4) {{
        { 1,   0,  0,  0 },
        { 0,   1,  0,  0 },
        { 0,   0,  1,  0 },
        { tx, ty, tz,  1 }
    }};
}

static matrix_float4x4 matrix4x4_rotation_z(float radians) {
    return (matrix_float4x4) {{
        {  cosf(radians),  sinf(radians),  0,  0 },
        { -sinf(radians),  cosf(radians),  0,  0 },
        {              0,              0,  1,  0 },
        {              0,              0,  0,  1 }
    }};

}

matrix_float4x4 matrix_perspective_right_hand(float fovyRadians, float aspect, float nearZ, float farZ) {
    float ys = 1 / tanf(fovyRadians * 0.5);
    float xs = ys / aspect;
    float zs = farZ / (nearZ - farZ);

    return (matrix_float4x4) {{
        { xs,   0,          0,  0 },
        {  0,  ys,          0,  0 },
        {  0,   0,         zs, -1 },
        {  0,   0, nearZ * zs,  0 }
    }};
}

matrix_float4x4 matrix_ortho(float l, float r, float b, float t, float n, float f) {
    return (matrix_float4x4) {{
        {2.0f / (r - l), 0, 0, 0},
        {0, 2.0f / (t - b), 0, 0},
        {0, 0, 1.0f / (f - n), 0},
        {(l + r) / (l - r), (t + b) / (b - t), n / (n - f), 1}
    }};
}
//=====================================================================
//=====================================================================






//=====================================================================
// Platform ios implementation
//=====================================================================

File os_file_read(struct Arena *arena, char *path) {
    File file = {0};

    char file_name[1024];
    char file_ext[1024];
    
    
    i32 path_legth = (i32)strlen(path);
    
    char *letter = path;
    i32 point_offset = 0;
    for(; *letter != '.'; letter++, point_offset++);
    

    memcpy(file_name, path, point_offset);
    point_offset++;
    memcpy(file_ext, path + point_offset, path_legth - point_offset);

    file_name[point_offset] = 0;
    file_ext[path_legth - point_offset] = 0;
    
    NSString *file_path = [[NSBundle mainBundle] pathForResource: [NSString stringWithUTF8String:file_name] 
                                                          ofType: [NSString stringWithUTF8String:file_ext]];
    if(file_path != nil) {
    }


    return file;
}

bool os_file_write(u8 *data, sz size) {
    return false;
}

Gpu gpu_load(struct Arena *arena) {

    IosRenderer *renderer = (IosRenderer *)arena_push(arena, sizeof(*renderer), 8);
    renderer->view = tmp_view;
    renderer->device = tmp_view.device;
    renderer->command_queue = [renderer->device newCommandQueue];
    tmp_view = nil;
    
    renderer->in_flight_semaphore = dispatch_semaphore_create(MAX_FRAMES_IN_FLIGHT);
    renderer->current_buffer = 0;


    id<MTLLibrary> shaderLib = [renderer->device newDefaultLibrary];
    if(!shaderLib) {
        NSLog(@"Error: couldnt create a default shader library");
        return nil;
    }
    
    id<MTLFunction> vertexProgram = [shaderLib newFunctionWithName:@"vertexShader"];
    if(!vertexProgram) {
        NSLog(@"Error: couldnt load vertex function from shader lib");
        return nil;
    }
    
    id<MTLFunction> fragmentProgram = [shaderLib newFunctionWithName:@"fragmentShader"];
    if(!fragmentProgram) {
        NSLog(@"Error: couldnt load fragment function from shader lib");
        return nil;
    }



    NSError *error;

    MTLRenderPipelineDescriptor *alpha_blend_state_desc = [[MTLRenderPipelineDescriptor alloc] init];
    alpha_blend_state_desc.label = @"MyPipeline";
    alpha_blend_state_desc.vertexFunction = vertexProgram;
    alpha_blend_state_desc.fragmentFunction = fragmentProgram;
    alpha_blend_state_desc.colorAttachments[0].pixelFormat = renderer->view.colorPixelFormat;
    alpha_blend_state_desc.colorAttachments[0].blendingEnabled = true;
    alpha_blend_state_desc.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    alpha_blend_state_desc.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    alpha_blend_state_desc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    alpha_blend_state_desc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    alpha_blend_state_desc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    alpha_blend_state_desc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

    renderer->alpha_blend_state = [renderer->device newRenderPipelineStateWithDescriptor:alpha_blend_state_desc error:&error];
    if(!renderer->alpha_blend_state) {
        NSLog(@"Error: Failed aquiring pipeline state: %@", error);
        return nil;
    }

    MTLRenderPipelineDescriptor *additive_blend_state_desc = [[MTLRenderPipelineDescriptor alloc] init];
    additive_blend_state_desc.label = @"MyPipeline";
    additive_blend_state_desc.vertexFunction = vertexProgram;
    additive_blend_state_desc.fragmentFunction = fragmentProgram;
    additive_blend_state_desc.colorAttachments[0].pixelFormat = renderer->view.colorPixelFormat;
    additive_blend_state_desc.colorAttachments[0].blendingEnabled = true;
    additive_blend_state_desc.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    additive_blend_state_desc.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    additive_blend_state_desc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    additive_blend_state_desc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    additive_blend_state_desc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    additive_blend_state_desc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

    renderer->additive_blend_state = [renderer->device newRenderPipelineStateWithDescriptor:additive_blend_state_desc error:&error];
    if(!renderer->additive_blend_state) {
        NSLog(@"Error: Failed aquiring pipeline state: %@", error);
        return nil;
    }

    
    renderer->buffer_index = 0;
    renderer->quad_count = 0;
    renderer->vbuffer = [renderer->device newBufferWithBytes:&quad_vertices
                                                      length:(sizeof(Vertex) * 6)
                                                     options:MTLResourceStorageModeShared];

    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        for(int j = 0; j < MAX_UBUFFER_COUNT; j++) {
            renderer->ubuffer[i][j] = [renderer->device newBufferWithLength:MAX_QUAD_COUNT * sizeof(Uniform)
                                                                    options:MTLResourceCPUCacheModeDefaultCache];
        }
    }

    f32 hw = (f32)renderer->view.bounds.size.width * 0.5f;
    f32 hh = (f32)renderer->view.bounds.size.height * 0.5f;
    renderer->proj_m4 = matrix_ortho(-hw, hw, -hh, hh, 0, -100.0f);
    renderer->view_m4 = matrix4x4_translation(0, 0, 0);

    return (Gpu)renderer;
}

void gpu_unload(Gpu gpu) {
    IosRenderer *renderer = (IosRenderer *)gpu;

}

void gpu_frame_begin(Gpu gpu) {
    IosRenderer *renderer = (IosRenderer *)gpu;

    dispatch_semaphore_wait(renderer->in_flight_semaphore, DISPATCH_TIME_FOREVER);
    renderer->current_buffer = (renderer->current_buffer + 1) % MAX_FRAMES_IN_FLIGHT;
    renderer->buffer_index = 0;
    renderer->quad_count = 0;

    MTLRenderPassDescriptor *render_pass_descriptor = renderer->view.currentRenderPassDescriptor;
    if (render_pass_descriptor == nil) {
        return;
    }
    renderer->command_buffer = [renderer->command_queue commandBuffer];
    renderer->command_encoder = [renderer->command_buffer renderCommandEncoderWithDescriptor:render_pass_descriptor];

    [renderer->command_encoder setRenderPipelineState: renderer->alpha_blend_state];
    [renderer->command_encoder setVertexBuffer: renderer->vbuffer offset:0 atIndex:VertexInputIndexVertices];
    [renderer->command_encoder setVertexBytes:&renderer->view_m4 length:sizeof(matrix_float4x4) atIndex:VertexInputIndexView];
    [renderer->command_encoder setVertexBytes:&renderer->proj_m4 length:sizeof(matrix_float4x4) atIndex:VertexInputIndexProj];


}

void gpu_frame_end(Gpu gpu) {
    IosRenderer *renderer = (IosRenderer *)gpu;
    if(renderer->quad_count > 0) {
        [renderer->command_encoder setVertexBuffer:renderer->ubuffer[renderer->current_buffer][renderer->buffer_index]
                                            offset:0
                                           atIndex:VertexInputIndexWorld];
        [renderer->command_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6 instanceCount:renderer->quad_count];
        renderer->quad_count = 0;
    }

    [renderer->command_encoder endEncoding];
    id<MTLDrawable> drawable = renderer->view.currentDrawable;
    [renderer->command_buffer presentDrawable:drawable]; 

    __block dispatch_semaphore_t block_semaphore = renderer->in_flight_semaphore;
    [renderer->command_buffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        dispatch_semaphore_signal(block_semaphore);
    }];

    [renderer->command_buffer commit];
}

Texture gpu_texture_load(Bitmap *bitmap) {
    return 0;
}

void gpu_texture_unload(Texture texture) {

}

void gpu_shader_set(void) {

}

void gpu_draw_quad_texture(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle, Texture texture) {
    IosRenderer *renderer = (IosRenderer *)gpu;
    matrix_float4x4 trans = matrix4x4_translation(x, y, -20);
    matrix_float4x4 rot = matrix4x4_rotation_z(angle);
    matrix_float4x4 scale = matrix4x4_scale(w, h, 1);
    matrix_float4x4 world = matrix_multiply(trans, matrix_multiply(rot, scale));

    if(renderer->buffer_index == MAX_UBUFFER_COUNT) {
        return;
    }

    Uniform *dst = renderer->ubuffer[renderer->current_buffer][renderer->buffer_index].contents + (sizeof(Uniform) * renderer->quad_count);
    memcpy(&dst->world, &world, sizeof(matrix_float4x4));
    dst->texture_id = 1;
    renderer->quad_count++;

    if(renderer->quad_count == MAX_QUAD_COUNT) {
        [renderer->command_encoder setVertexBuffer:renderer->ubuffer[renderer->current_buffer][renderer->buffer_index]
                                            offset:0
                                           atIndex:VertexInputIndexWorld];
        [renderer->command_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6 instanceCount:renderer->quad_count];
        renderer->quad_count = 0;
        renderer->buffer_index++;
    }
}

void gpu_draw_quad_color(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle, V3 color) {
    IosRenderer *renderer = (IosRenderer *)gpu;
    matrix_float4x4 trans = matrix4x4_translation(x, y, -20);
    matrix_float4x4 rot = matrix4x4_rotation_z(angle);
    matrix_float4x4 scale = matrix4x4_scale(w, h, 1);
    matrix_float4x4 world = matrix_multiply(trans, matrix_multiply(rot, scale));

    if(renderer->buffer_index == MAX_UBUFFER_COUNT) {
        return;
    }

    Uniform *dst = renderer->ubuffer[renderer->current_buffer][renderer->buffer_index].contents + (sizeof(Uniform) * renderer->quad_count);
    memcpy(&dst->world, &world, sizeof(matrix_float4x4));
    dst->texture_id = 0;
    renderer->quad_count++;

    if(renderer->quad_count == MAX_QUAD_COUNT) {
        [renderer->command_encoder setVertexBuffer:renderer->ubuffer[renderer->current_buffer][renderer->buffer_index]
                                            offset:0
                                           atIndex:VertexInputIndexWorld];
        [renderer->command_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6 instanceCount:renderer->quad_count];
        renderer->quad_count = 0;
        renderer->buffer_index++;
    }
}

void gpu_camera_set(V3 pos, f32 angle) {

}
//=====================================================================
//=====================================================================




//=====================================================================
// AppDelegate
//=====================================================================
@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@end

@implementation AppDelegate
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    return YES;
}

- (void)applicationWillTerminate:(UIApplication *)application {
    game_shutdown(&g_memory);
}
@end
//=====================================================================
//=====================================================================





//=====================================================================
// MTKViewDelegate
//=====================================================================
@interface MetalViewDelegate : NSObject<MTKViewDelegate>
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view;
@end

@implementation MetalViewDelegate

- (nonnull instancetype) initWithMetalKitView:(nonnull MTKView *)view {
    self = [super init];
    return self;   
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    game_update(&g_memory, 0, 0);
    game_render(&g_memory);
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {

}
@end
//=====================================================================
//=====================================================================







//=====================================================================
// ViewController
//=====================================================================
@interface ViewController : UIViewController
@end

@implementation ViewController {
    MetalViewDelegate *_metal_view_delegate;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    tmp_view = (MTKView *)self.view;
    tmp_view.device = MTLCreateSystemDefaultDevice();
    tmp_view.clearColor = MTLClearColorMake(0.0, 0.5, 1.0, 1.0);
    
    _metal_view_delegate = [[MetalViewDelegate alloc] initWithMetalKitView:tmp_view];
    if(!_metal_view_delegate) {
        NSLog(@"Renderer initialization failed");
        return;
    }
    [_metal_view_delegate mtkView:tmp_view drawableSizeWillChange:tmp_view.drawableSize];
    tmp_view.delegate = _metal_view_delegate;    

    g_memory.size = mb(256);
    g_memory.used = 0;
    g_memory.data = malloc(g_memory.size);
    game_init(&g_memory);

}

@end
//=====================================================================
//=====================================================================






//=====================================================================
// Main Entry point of the program
//=====================================================================
int main(int argc, char * argv[]) {
    NSString * appDelegateClassName;
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    }
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
//=====================================================================
//=====================================================================
