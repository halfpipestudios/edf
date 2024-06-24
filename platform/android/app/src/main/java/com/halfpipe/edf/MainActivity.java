package com.halfpipe.edf;

import android.annotation.SuppressLint;
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
            getWindow().getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
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
    int index;
    float x, y;
}

class GameInput {

    static final int MAX_TOUCHES = 10;

    Touch[] touches;
    int[] indices;
    int indices_count;

    GameInput() {
        touches = new Touch[MAX_TOUCHES];
        indices = new int[MAX_TOUCHES];

        for(int i = 0; i < touches.length; ++i) {
            touches[i] = new Touch();
        }

        indices_count = 0;
    }

}

class GameView extends GLSurfaceView {

    private final GameRenderer renderer;
    private GameInput input;

    public GameView(Context context) {
        super(context);

        setEGLContextClientVersion(3);
        setEGLConfigChooser(8, 8, 8, 8, 24, 8);

        input = new GameInput();
        renderer = new GameRenderer(context.getAssets(), input);

        setRenderer(renderer);

        setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);


    }

    private void updateTouch(MotionEvent event, int eventType, int action_index) {
        int index = event.getPointerId(action_index);
        input.touches[index].event = eventType;
        input.touches[index].x = event.getX(action_index);
        input.touches[index].y = event.getY(action_index);
        input.touches[index].index = index;
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public boolean onTouchEvent(final MotionEvent event) {

        int action = event.getAction() &  MotionEvent.ACTION_MASK;
        int action_index = event.getActionIndex();

        switch (action) {

            case  MotionEvent.ACTION_DOWN:
            case  MotionEvent.ACTION_POINTER_DOWN: {
                updateTouch(event, Touch.TOUCH_EVENT_DOWN, action_index);
                int index = event.getPointerId(action_index);
                input.indices[input.indices_count++] = index;
            } break;

            case  MotionEvent.ACTION_UP:
            case  MotionEvent.ACTION_POINTER_UP:
            case MotionEvent.ACTION_CANCEL: {
                updateTouch(event, Touch.TOUCH_EVENT_UP, action_index);
                int index = event.getPointerId(action_index);
                input.indices[index] = input.indices[input.indices_count-1];
                --input.indices_count;
            } break;

            case  MotionEvent.ACTION_MOVE: {
                for(int i = 0; i < input.indices_count; ++i) {
                    updateTouch(event, Touch.TOUCH_EVENT_MOVE, i);
                }
            } break;

        }

        return true;
    }
}

class GameRenderer implements GLSurfaceView.Renderer {

    public native void gameInit(AssetManager assetManager);

    public native void gameUpdate(int count, int[] indices, Touch[] touches, float dt);

    public native void gameRender();

    public native void gpuSetViewport(int x, int y, int w, int h);

    private static final double NANOS_PER_SECOND = 1000000000.0;
    private long lastTime;
    AssetManager assetManager;
    private GameInput input;

    public GameRenderer(AssetManager assetManager, GameInput input) {
        this.assetManager = assetManager;
        this.input = input;
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
        gameUpdate(input.indices_count, input.indices, input.touches, (float) dt);
        gameRender();
    }

}