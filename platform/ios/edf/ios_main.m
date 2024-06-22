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

// TODO(manu): add IOS_ prefix to this defines
#define MAX_FRAMES_IN_FLIGHT 3
#define MAX_UBUFFER_COUNT 1
#define MAX_QUAD_COUNT 10000

#define MAX_TEXTURE_COUNT 200
#define TEXTURE_SIZE_8x8 0
#define TEXTURE_SIZE_16x16 1
#define TEXTURE_SIZE_32x32 2
#define TEXTURE_SIZE_64x64 3
#define TEXTURE_SIZE_COUNT 4

static const Vertex quad_vertices[] = {
    { {  0.5,  -0.5 }, {1.0, 1.0} },
    { { -0.5,  -0.5 }, {0.0, 1.0} },
    { { -0.5,   0.5 }, {0.0, 0.0} },
    { {  0.5,  -0.5 }, {1.0, 1.0} },
    { { -0.5,   0.5 }, {0.0, 0.0} },
    { {  0.5,   0.5 }, {1.0, 0.0} }
};

typedef struct IosTexture {
    i32 array_index;
    i32 texture_index;
    f32 u_ratio;
    f32 v_ratio;
} IosTexture;


typedef struct IosTextureArray {
    id<MTLTexture> array;
    IosTexture textures[MAX_TEXTURE_COUNT];
    i32 index_array[MAX_TEXTURE_COUNT];
    i32 free_list;
    u32 count;
} IosTextureArray;

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
    
    GpuBlendState blend_state;
    // TODO(manu): create a struct for this
    id<MTLBuffer> ubuffer[2][MAX_FRAMES_IN_FLIGHT][MAX_UBUFFER_COUNT];
    u32 quad_count[2];
    u32 buffer_index[2];

    matrix_float4x4 proj_m4;
    matrix_float4x4 view_m4;
    
    IosTextureArray texture_arrays[TEXTURE_SIZE_COUNT]; 

} IosRenderer;


// globals variables
static Memory g_memory;
static u32 g_view_width;
static u32 g_view_height;
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


void ios_texture_array_init(IosRenderer *renderer, IosTextureArray *texture_array, i32 width, i32 height) {
    MTLTextureDescriptor *texture_descriptor;
    texture_descriptor = [[MTLTextureDescriptor alloc] init];
    texture_descriptor.pixelFormat = MTLPixelFormatRGBA8Unorm;
    texture_descriptor.width = width;
    texture_descriptor.height = height;
    texture_descriptor.arrayLength = MAX_TEXTURE_COUNT;
    texture_descriptor.textureType = MTLTextureType2DArray;
    texture_array->array = [renderer->device newTextureWithDescriptor: texture_descriptor];
    texture_array->count = 0;
    texture_array->free_list = 0;
    for(i32 i = 0; i < MAX_TEXTURE_COUNT - 1; i++) {
        texture_array->index_array[i] = i + 1;
    }
    texture_array->index_array[MAX_TEXTURE_COUNT - 1] = -1;
}


//=====================================================================
//=====================================================================






//=====================================================================
// Platform ios implementation
//=====================================================================

File os_file_read(struct Arena *arena, char *path) {
    File file = {0};
    
    char file_header[7] = "assets/";
    char file_name[1024];
    char file_ext[1024];
     
    i32 path_legth = (i32)strlen(path);
    
    char *letter = path;
    i32 point_offset = 0;
    for(; *letter != '.'; letter++, point_offset++);
    
    memcpy(file_name, file_header, 7);
    memcpy(file_name + 7, path, point_offset);
    file_name[point_offset + 7] = 0;
    
    point_offset++;
    
    memcpy(file_ext, path + point_offset, path_legth - point_offset);
    file_ext[path_legth - point_offset] = 0;
    
    NSString *file_path = [[NSBundle mainBundle] pathForResource: [NSString stringWithUTF8String:file_name]
                                                          ofType: [NSString stringWithUTF8String:file_ext]];
    
    FILE *file_handle = fopen([file_path cStringUsingEncoding:NSUTF8StringEncoding], "rb");
    if(!file_handle) {
        return file;
    }
    // go to the end of the file
    fseek(file_handle, 0, SEEK_END);
    // get the size of the file to alloc the memory we need
    long int file_size = ftell(file_handle);
    // go back to the start of the file
    fseek(file_handle, 0, SEEK_SET);
    // alloc the memory
    file.data = arena_push(arena, file_size + 1, 8);
    memset(file.data, 0, file_size + 1);
    // store the content of the file
    fread(file.data, file_size, 1, file_handle);
    fclose(file_handle);

    file.size = file_size;

    return file;
}

bool os_file_write(u8 *data, sz size) {
    return false;
}

u32 os_display_width(void) {
    return g_view_width;
}

u32 os_display_height(void) {
    return g_view_height;
}


void os_print(char *message, ...) {
    char buffer[32000];
    va_list valist;
    va_start(valist, message);
    vsnprintf(buffer, array_len(buffer), message, valist);
    va_end(valist);
    NSLog(@"%@", [NSString stringWithUTF8String:buffer]);
}

Gpu gpu_load(struct Arena *arena) {

    IosRenderer *renderer = (IosRenderer *)arena_push(arena, sizeof(*renderer), 8);
    renderer->view = tmp_view;
    renderer->device = tmp_view.device;
    renderer->command_queue = [renderer->device newCommandQueue];
    
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
    additive_blend_state_desc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    additive_blend_state_desc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOne;
    additive_blend_state_desc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;

    renderer->additive_blend_state = [renderer->device newRenderPipelineStateWithDescriptor:additive_blend_state_desc error:&error];
    if(!renderer->additive_blend_state) {
        NSLog(@"Error: Failed aquiring pipeline state: %@", error);
        return nil;
    }

    renderer->blend_state = GPU_BLEND_STATE_ALPHA;
    renderer->buffer_index[0] = 0;
    renderer->buffer_index[1] = 0;
    renderer->quad_count[0] = 0;
    renderer->quad_count[1] = 0;
    renderer->vbuffer = [renderer->device newBufferWithBytes:&quad_vertices
                                                      length:(sizeof(Vertex) * 6)
                                                     options:MTLResourceStorageModeShared];
    
    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        for(int j = 0; j < MAX_UBUFFER_COUNT; j++) {
            renderer->ubuffer[0][i][j] = [renderer->device newBufferWithLength:MAX_QUAD_COUNT * sizeof(Uniform)
                                                                       options:MTLResourceCPUCacheModeDefaultCache];
            renderer->ubuffer[1][i][j] = [renderer->device newBufferWithLength:MAX_QUAD_COUNT * sizeof(Uniform)
                                                                       options:MTLResourceCPUCacheModeDefaultCache];
        }
    }

    ios_texture_array_init(renderer, &renderer->texture_arrays[TEXTURE_SIZE_8x8], 8, 8);
    ios_texture_array_init(renderer, &renderer->texture_arrays[TEXTURE_SIZE_16x16], 16, 16);
    ios_texture_array_init(renderer, &renderer->texture_arrays[TEXTURE_SIZE_32x32], 32, 32);
    ios_texture_array_init(renderer, &renderer->texture_arrays[TEXTURE_SIZE_64x64], 64, 64);

    u32 white[8*8];
    for(u32 i = 0; i < 8*8; i++) {
        white[i] = 0xFFFFFFFF;
    }

    Bitmap white_bitmap;
    white_bitmap.data = white;
    white_bitmap.width = 8;
    white_bitmap.height = 8;
    gpu_texture_load((void *)renderer, &white_bitmap);

    renderer->view_m4 = matrix4x4_translation(0, 0, 0);
    
    return (Gpu)renderer;
}

void gpu_unload(Gpu gpu) {
    IosRenderer *renderer = (IosRenderer *)gpu;

}

void gpu_frame_begin(Gpu gpu) {
    IosRenderer *renderer = (IosRenderer *)gpu;
    
    renderer->blend_state = GPU_BLEND_STATE_ALPHA;
    
    dispatch_semaphore_wait(renderer->in_flight_semaphore, DISPATCH_TIME_FOREVER);
    renderer->current_buffer = (renderer->current_buffer + 1) % MAX_FRAMES_IN_FLIGHT;
    renderer->buffer_index[0] = 0;
    renderer->buffer_index[1] = 0;
    renderer->quad_count[0] = 0;
    renderer->quad_count[1] = 0;


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
    [renderer->command_encoder setFragmentTexture:renderer->texture_arrays[TEXTURE_SIZE_8x8].array atIndex:TEXTURE_SIZE_8x8];
    [renderer->command_encoder setFragmentTexture:renderer->texture_arrays[TEXTURE_SIZE_16x16].array atIndex:TEXTURE_SIZE_16x16];
    [renderer->command_encoder setFragmentTexture:renderer->texture_arrays[TEXTURE_SIZE_32x32].array atIndex:TEXTURE_SIZE_32x32];
    [renderer->command_encoder setFragmentTexture:renderer->texture_arrays[TEXTURE_SIZE_64x64].array atIndex:TEXTURE_SIZE_64x64];
}

void gpu_frame_end(Gpu gpu) {
    IosRenderer *renderer = (IosRenderer *)gpu;
    if(renderer->quad_count[0] > 0) {
        [renderer->command_encoder setRenderPipelineState: renderer->alpha_blend_state];
        [renderer->command_encoder setVertexBuffer:renderer->ubuffer[0][renderer->current_buffer][renderer->buffer_index[0]]
                                            offset:0
                                           atIndex:VertexInputIndexWorld];
        [renderer->command_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6 instanceCount:renderer->quad_count[0]];
        renderer->quad_count[0] = 0;
    }
    
    if(renderer->quad_count[1] > 0) {
        [renderer->command_encoder setRenderPipelineState: renderer->additive_blend_state];
        [renderer->command_encoder setVertexBuffer:renderer->ubuffer[1][renderer->current_buffer][renderer->buffer_index[1]]
                                            offset:0
                                           atIndex:VertexInputIndexWorld];
        [renderer->command_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6 instanceCount:renderer->quad_count[1]];
        renderer->quad_count[1] = 0;
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

Texture gpu_texture_load(Gpu gpu, Bitmap *bitmap) {
    IosRenderer *renderer = (IosRenderer *)gpu;
    
    i32 gpu_texture_sizes[TEXTURE_SIZE_COUNT] = {
        8, 16, 32, 64
    };

    i32 bitmap_size = max(bitmap->width, bitmap->height);
    i32 array_index = TEXTURE_SIZE_8x8;
    if(bitmap_size > 8) {
        array_index = TEXTURE_SIZE_16x16;
        if(bitmap_size > 16) {
            array_index = TEXTURE_SIZE_32x32;
            if(bitmap_size > 32) {
                array_index = TEXTURE_SIZE_64x64;
                if(bitmap_size > 64) {
                    assert(!"texture bigger that 64x64 are not supported!");
                }
            }
        } 
    }

    IosTextureArray *array = renderer->texture_arrays + array_index;
    if(array->count + 1 >= MAX_TEXTURE_COUNT) {
        return (void *)-1;
    }

    // find the next free texture index
    i32 free_index = array->free_list;
    array->free_list = array->index_array[free_index];
    IosTexture *texture = &array->textures[free_index];
    array->index_array[free_index] = -1;

    NSUInteger bytes_per_row = 4 * bitmap->width;
    MTLRegion region = {
        { 0, 0, 0 },
        {bitmap->width, bitmap->height, 1}
    };
    // Copy the bytes from the data object into the texture
    [array->array replaceRegion:region
                    mipmapLevel:0
                          slice:free_index
                      withBytes:bitmap->data
                    bytesPerRow:bytes_per_row
                  bytesPerImage:0];

    
    texture->u_ratio = (f32)bitmap->width / (f32)gpu_texture_sizes[array_index];
    texture->v_ratio = (f32)bitmap->height / (f32)gpu_texture_sizes[array_index];
    texture->array_index = array_index;
    texture->texture_index = free_index;
    
    array->count++;
    
    return (Texture)texture;
}

void gpu_texture_unload(Gpu gpu, Texture texture) {
    IosRenderer *renderer = (IosRenderer *)gpu;
    
    IosTexture *ios_texture = (IosTexture *)texture;
    
    i32 array_index = ios_texture->array_index;
    i32 texture_index_to_free = ios_texture->texture_index;
    
    IosTextureArray *array = renderer->texture_arrays + array_index;

    array->index_array[texture_index_to_free] = array->free_list;
    array->free_list = texture_index_to_free;
    array->count--;
}


void gpu_blend_state_set(Gpu gpu, GpuBlendState blend_state) {
    IosRenderer *renderer = (IosRenderer *)gpu;
    renderer->blend_state = blend_state;
}

void gpu_draw_quad_texture(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle, Texture texture) {
    IosRenderer *renderer = (IosRenderer *)gpu;

    IosTexture *ios_texture = (IosTexture *)texture;

    matrix_float4x4 trans = matrix4x4_translation(x, y, -20);
    matrix_float4x4 rot = matrix4x4_rotation_z(angle);
    matrix_float4x4 scale = matrix4x4_scale(w, h, 1);
    matrix_float4x4 world = matrix_multiply(trans, matrix_multiply(rot, scale));
    
    u32 i = renderer->blend_state;
    if(renderer->buffer_index[i] == MAX_UBUFFER_COUNT) {
        return;
    }
    Uniform *dst = renderer->ubuffer[i][renderer->current_buffer][renderer->buffer_index[i]].contents + (sizeof(Uniform) * renderer->quad_count[i]);
    memcpy(&dst->world, &world, sizeof(matrix_float4x4));
    dst->u_ratio = ios_texture->u_ratio;
    dst->v_ratio = ios_texture->v_ratio;
    dst->array_index = ios_texture->array_index;
    dst->texture_index = ios_texture->texture_index;
    dst->color.x = 1.0;
    dst->color.y = 1.0;
    dst->color.z = 1.0;
    renderer->quad_count[i]++;

    if(renderer->quad_count[i] == MAX_QUAD_COUNT) {
        if(i == GPU_BLEND_STATE_ALPHA) {
            [renderer->command_encoder setRenderPipelineState: renderer->alpha_blend_state];
        }
        else {
            [renderer->command_encoder setRenderPipelineState: renderer->additive_blend_state];
        }
        [renderer->command_encoder setVertexBuffer:renderer->ubuffer[i][renderer->current_buffer][renderer->buffer_index[i]]
                                            offset:0
                                           atIndex:VertexInputIndexWorld];
        [renderer->command_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6 instanceCount:renderer->quad_count[i]];
        renderer->quad_count[i] = 0;
        renderer->buffer_index[i]++;
    }
}

void gpu_draw_quad_color(Gpu gpu, f32 x, f32 y, f32 w, f32 h, f32 angle, V3 color) {
    IosRenderer *renderer = (IosRenderer *)gpu;
    matrix_float4x4 trans = matrix4x4_translation(x, y, -20);
    matrix_float4x4 rot = matrix4x4_rotation_z(angle);
    matrix_float4x4 scale = matrix4x4_scale(w, h, 1);
    matrix_float4x4 world = matrix_multiply(trans, matrix_multiply(rot, scale));

    u32 i = renderer->blend_state;
    if(renderer->buffer_index[i] == MAX_UBUFFER_COUNT) {
        return;
    }
    Uniform *dst = renderer->ubuffer[i][renderer->current_buffer][renderer->buffer_index[i]].contents + (sizeof(Uniform) * renderer->quad_count[i]);
    memcpy(&dst->world, &world, sizeof(matrix_float4x4));
    dst->u_ratio = 1.0;
    dst->v_ratio = 1.0;
    dst->array_index = 0;
    dst->texture_index = 0;
    dst->color.x = color.x;
    dst->color.y = color.y;
    dst->color.z = color.z;
    renderer->quad_count[i]++;

    if(renderer->quad_count[i] == MAX_QUAD_COUNT) {
        if(i == GPU_BLEND_STATE_ALPHA) {
            [renderer->command_encoder setRenderPipelineState: renderer->alpha_blend_state];
        }
        else {
            [renderer->command_encoder setRenderPipelineState: renderer->additive_blend_state];
        }
        [renderer->command_encoder setVertexBuffer:renderer->ubuffer[i][renderer->current_buffer][renderer->buffer_index[i]]
                                            offset:0
                                           atIndex:VertexInputIndexWorld];
        [renderer->command_encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6 instanceCount:renderer->quad_count[i]];
        renderer->quad_count[i] = 0;
        renderer->buffer_index[i]++;
    }
}

void gpu_camera_set(Gpu gpu, V3 pos, f32 angle) {
    IosRenderer *renderer = (IosRenderer *)gpu;
    renderer->view_m4 = matrix4x4_translation(-pos.x, -pos.y, -pos.z);
}


void gpu_resize(Gpu gpu, u32 w, u32 h) {
    IosRenderer *renderer = (IosRenderer *)gpu;
    f32 hw = (f32)w * 0.5f;
    f32 hh = (f32)h * 0.5f;
    renderer->proj_m4 = matrix_ortho(-hw, hw, -hh, hh, 0, -100.0f);
    
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
    game_update(&g_memory, 0.016);
    game_render(&g_memory);
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    g_view_width = size.width;
    g_view_height = size.height;
    game_resize(&g_memory, size.width, size.height);
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
    tmp_view.clearColor = MTLClearColorMake(0.2, 0.5, 0.7, 1.0);
    
    _metal_view_delegate = [[MetalViewDelegate alloc] initWithMetalKitView:tmp_view];
    if(!_metal_view_delegate) {
        NSLog(@"Renderer initialization failed");
        return;
    }

    g_memory.size = mb(256);
    g_memory.used = 0;
    g_memory.data = malloc(g_memory.size);
    
    tmp_view.delegate = _metal_view_delegate;
    
    game_init(&g_memory);

    [_metal_view_delegate mtkView:tmp_view drawableSizeWillChange:tmp_view.drawableSize];
    
    tmp_view = nil;
}

- (void) touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    assert(touches.count <= MAX_TOUCHES);
    Input input;
    input.touches_count = touches.count;
    for(i32 i = 0; i < input.touches_count; i++) {
        UITouch *uitouch = touches.allObjects[i]; 
        CGPoint location = [uitouch locationInView:self.view];
        Touch *touch = input.touches + i; 
        touch->pos.x = location.x;
        touch->pos.y = location.y;
    } 
    game_touches_down(&g_memory, &input);
}

- (void) touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {  
    assert(touches.count <= MAX_TOUCHES);
    Input input;
    input.touches_count = touches.count;
    for(i32 i = 0; i < input.touches_count; i++) {
        UITouch *uitouch = touches.allObjects[i]; 
        CGPoint location = [uitouch locationInView:self.view];
        Touch *touch = input.touches + i; 
        touch->pos.x = location.x;
        touch->pos.y = location.y;
    } 
    game_touches_move(&g_memory, &input);
}

- (void) touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    assert(touches.count <= MAX_TOUCHES);
    Input input;
    input.touches_count = touches.count;
    for(i32 i = 0; i < input.touches_count; i++) {
        UITouch *uitouch = touches.allObjects[i]; 
        CGPoint location = [uitouch locationInView:self.view];
        Touch *touch = input.touches + i; 
        touch->pos.x = location.x;
        touch->pos.y = location.y;
    } 
    game_touches_up(&g_memory, &input);
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
