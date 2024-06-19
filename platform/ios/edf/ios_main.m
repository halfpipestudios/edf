//
//  main.m
//  edf
//
//  Created by Manuel Cabrerizo on 18/06/2024.
//


#include "edf_platform.h"
#include "edf_memory.h"
#include "edf.h"

#include <simd/simd.h>
#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import<MetalKit/MetalKit.h>

#define MAX_FRAMES_IN_FLIGHT 3
#define MAX_UBUFFER_COUNT 3
#define MAX_QUAD_COUNT 10000

typedef struct IosRenderer {
    id<MTLCommandBuffer> command_buffer;
    id<MTLRenderCommandEncoder> command_encoder;
} IosRenderer;


static MTKView *g_view;
static id<MTLDevice> g_device;
static id<MTLCommandQueue> g_command_queue;
static Memory g_memory;


//=====================	================================================
// Platform ios implementation
//=====================================================================
Gpu gpu_load(struct Arena *arena) {
    IosRenderer *renderer = (IosRenderer *)arena_push(arena, sizeof(*renderer), 8);
    return (Gpu)renderer;
}

void gpu_unload(Gpu gpu) {
    IosRenderer *renderer = (IosRenderer *)gpu;

}

void gpu_frame_begin(Gpu gpu) {
    IosRenderer *renderer = (IosRenderer *)gpu;
    MTLRenderPassDescriptor *render_pass_descriptor = g_view.currentRenderPassDescriptor;
    if (render_pass_descriptor == nil) {
        return;
    }
    renderer->command_buffer = [g_command_queue commandBuffer];
    renderer->command_encoder = [renderer->command_buffer renderCommandEncoderWithDescriptor:render_pass_descriptor];
}

void gpu_frame_end(Gpu gpu) {
    IosRenderer *renderer = (IosRenderer *)gpu;
    [renderer->command_encoder endEncoding];
    id<MTLDrawable> drawable = g_view.currentDrawable;
    [renderer->command_buffer presentDrawable:drawable]; 
    [renderer->command_buffer commit];
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
    if(self)
    {
        g_device = view.device;
        g_command_queue = [g_device newCommandQueue];
    }

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

    g_view = (MTKView *)self.view;
    g_view.device = MTLCreateSystemDefaultDevice();
    g_view.clearColor = MTLClearColorMake(0.0, 0.5, 1.0, 1.0);
    
    _metal_view_delegate = [[MetalViewDelegate alloc] initWithMetalKitView:g_view];
    if(!_metal_view_delegate) {
        NSLog(@"Renderer initialization failed");
        return;
    }
    [_metal_view_delegate mtkView:g_view drawableSizeWillChange:g_view.drawableSize];
    g_view.delegate = _metal_view_delegate;    

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
