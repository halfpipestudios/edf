//
//  main.m
//  edf
//
//  Created by Manuel Cabrerizo on 18/06/2024.
//


#include "edf_platform.h"
#include "edf.h"
#include <simd/simd.h>

#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import<MetalKit/MetalKit.h>

#define MAX_FRAMES_IN_FLIGHT 3
#define MAX_UBUFFER_COUNT 3
#define MAX_QUAD_COUNT 10000

typedef struct IosRenderer {
    // synchronisation
    //dispatch_semaphore_t in_flight_semaphore;
    //u32 current_buffer;
    // metal
    id<MTLDevice> device;
    id<MTLCommandQueue> command_queue;
    MTKView *view;
    id<MTLCommandBuffer> command_buffer;
    id<MTLRenderCommandEncoder> command_encoder;

    //id<MTLRenderPipelineState> pipeline_state;
    //MTLRenderPassDescriptor *render_pass_descriptor;

    // instance renderer
    id<MTLBuffer> vbuffer;
    id<MTLBuffer> ubuffer[MAX_UBUFFER_COUNT][MAX_QUAD_COUNT];
} IosRenderer;

static IosRenderer renderer;

/*
void gpu_init(void) {

}

void gpu_shutdown(void) {

}

void gpu_frame_begin(void) {
    MTLRenderPassDescriptor *render_pass_descriptor = renderer.view.currentRenderPassDescriptor;
    if (render_pass_descriptor == nil) {
        return;
    }
    renderer.command_buffer = [renderer.command_queue commandBuffer];
    renderer.command_encoder = [renderer.command_buffer renderCommandEncoderWithDescriptor:render_pass_descriptor];
}

void gpu_frame_end(void) {
    [renderer.command_encoder endEncoding];
    id<MTLDrawable> drawable = renderer.view.currentDrawable;
    [renderer.command_buffer presentDrawable:drawable]; 
    [renderer.command_buffer commit];
}
*/

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
    game_shutdown(0);
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
        renderer.device = view.device;
        renderer.command_queue = [renderer.device newCommandQueue];
        renderer.view = view;
    }

    return self;   
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    game_update(0, 0, 0);
    game_render(0);
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
    MTKView *_view;
    MetalViewDelegate *_metal_view_delegate;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    _view = (MTKView *)self.view;
    _view.device = MTLCreateSystemDefaultDevice();
    _view.clearColor = MTLClearColorMake(0.0, 0.5, 1.0, 1.0);
    
    _metal_view_delegate = [[MetalViewDelegate alloc] initWithMetalKitView:_view];
    if(!_metal_view_delegate) {
        NSLog(@"Renderer initialization failed");
        return;
    }
    [_metal_view_delegate mtkView:_view drawableSizeWillChange:_view.drawableSize];
    _view.delegate = _metal_view_delegate;    

    game_init(0); 
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
