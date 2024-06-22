package com.halfpipe.edf;

import android.content.res.AssetManager;
import android.os.Build;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.app.Activity;

import android.content.Context;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends Activity {

    static {
        System.loadLibrary("edf");
    }

    private GameView gameView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            getWindow()
                    .getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }

        gameView = new GameView(this);
        setContentView(gameView);

        gameView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
    }

}

class Touch {

    static final int TOUCH_EVENT_DOWN = 0;
    static final int TOUCH_EVENT_MOVE = 1;
    static final int TOUCH_EVENT_UP = 2;

    int event;
    float x, y;
}

class GameInput {

    Touch[] touches;
    int touches_count;

    GameInput() {
        touches = new Touch[10];
        touches_count = 0;
    }

}

class GameView extends GLSurfaceView {

    private final GameRenderer renderer;
    private GameInput input;

    public native void gameTouchesDown(int touch_count, Touch[] touches);
    public native void gameTouchesUp(int touch_count, Touch[] touches);
    public native void gameTouchesMove(int touch_count, Touch[] touches);

    public GameView(Context context) {
        super(context);

        setEGLContextClientVersion(3);
        setEGLConfigChooser(8, 8, 8, 8, 24, 8);

        renderer = new GameRenderer(context.getAssets());
        setRenderer(renderer);

        setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        input = new GameInput();
    }

    private void updateInput(final MotionEvent event, int touchEvent) {
        input.touches_count = Math.min(10, event.getPointerCount());
        for(int i = 0; i < input.touches_count; ++i) {
            Touch touch = new Touch();
            touch.event = touchEvent;
            touch.x = event.getX(i);
            touch.y = event.getY(i);
            input.touches[i] = (touch);
        }
    }

    @Override
    public boolean onTouchEvent(final MotionEvent event) {

        int action = event.getAction() &  MotionEvent.ACTION_MASK;

        switch (action) {

            case  MotionEvent.ACTION_DOWN:
            case  MotionEvent.ACTION_POINTER_DOWN: {
                updateInput(event, Touch.TOUCH_EVENT_DOWN);
                gameTouchesDown(input.touches_count, input.touches);
            } break;

            case  MotionEvent.ACTION_UP:
            case  MotionEvent.ACTION_POINTER_UP: {
                updateInput(event, Touch.TOUCH_EVENT_UP);
                gameTouchesUp(input.touches_count, input.touches);
            } break;

            case  MotionEvent.ACTION_MOVE: {
                updateInput(event, Touch.TOUCH_EVENT_MOVE);
                gameTouchesMove(input.touches_count, input.touches);
            } break;

        }

        return true;
    }
}

class GameRenderer implements GLSurfaceView.Renderer {

    public native void gameInit(AssetManager assetManager);

    public native void gameUpdate(float dt);

    public native void gameRender();

    public native void gpuSetViewport(int x, int y, int w, int h);

    private static final double NANOS_PER_SECOND = 1000000000.0;
    private long lastTime;
    AssetManager assetManager;

    public GameRenderer(AssetManager assetManager) {
        this.assetManager = assetManager;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        lastTime = System.nanoTime();
        gameInit(assetManager);

    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        gpuSetViewport(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        long currentTime = System.nanoTime();
        double dt = (currentTime - lastTime) / NANOS_PER_SECOND;
        lastTime = currentTime;
        gameUpdate((float) dt);
        gameRender();
    }

}