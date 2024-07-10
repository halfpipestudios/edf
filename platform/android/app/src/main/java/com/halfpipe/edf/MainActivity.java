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

    private static final double NANOS_PER_SECOND = 1000000000.0;
    private long lastTime;

    private GameView gameView;
    private  GameRenderer renderer;
    private GameInput input;
    private AssetManager assetManager;
    private GameJNI jni;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        input = new GameInput();
        renderer = new GameRenderer(this);
        assetManager = getAssets();
        gameView = new GameView(this, renderer, input);
        jni = new GameJNI();

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            getWindow().getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }

        setContentView(gameView);
        gameView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
    }

    void onGameInit() {
        lastTime = System.nanoTime();
        jni.gameInit(assetManager);
    }

    void onGameUpdate() {
        long currentTime = System.nanoTime();
        double dt = (currentTime - lastTime) / NANOS_PER_SECOND;
        lastTime = currentTime;
        jni.gameUpdate(input.touches, (float)dt);
    }

    void onGameRender() {
        jni.gameRender();
    }

    void onGameResize(int x, int y, int w, int h) {
        jni.gameResize(x, y, w, h);
    }

    @Override
    protected void onPause() {
        super.onPause();

    }

    @Override
    protected void onResume() {
        super.onResume();
        input.reset();
        lastTime = System.nanoTime();
    }
}

class Touch {
    int uid;
    float x, y;
    boolean up, down;
}

class GameInput {

    static final int MAX_TOUCHES = 5;

    Touch[] touches;

    GameInput() {
        touches = new Touch[MAX_TOUCHES];
        for(int i = 0; i < touches.length; ++i) {
            touches[i] = new Touch();
        }
    }

    void reset() {
        for(int i = 0; i < GameInput.MAX_TOUCHES; ++i) {
            Touch touch = touches[i];
            touch.uid = 0;
            touch.x = 0;
            touch.y = 0;
            touch.up = true;
            touch.down = false;
        }
    }
}

class GameJNI {

    static {
        System.loadLibrary("edf");
    }

    public native void gameInit(AssetManager manager);
    public native void gameUpdate(Touch[] touches, float dt);
    public native void gameRender();
    public native void gameResize(int x, int y, int w, int h);
}

class GameView extends GLSurfaceView {

    static int uid;
    GameInput input;
    GameRenderer renderer;

    public GameView(Context context) {
        super(context);
    }

    public GameView(Context context, GameRenderer renderer, GameInput input) {
        super(context);

        this.input = input;
        this.renderer = renderer;

        setEGLContextClientVersion(3);
        setEGLConfigChooser(8, 8, 8, 8, 24, 8);

        setRenderer(renderer);
        setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        uid = 0;
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public boolean onTouchEvent(final MotionEvent event) {

        int action = event.getAction() &  MotionEvent.ACTION_MASK;
        int action_index = event.getActionIndex();
        int index = event.getPointerId(action_index);

        switch (action) {

            case  MotionEvent.ACTION_DOWN:
            case  MotionEvent.ACTION_POINTER_DOWN: {
                if(index >= GameInput.MAX_TOUCHES) return true;
                uid = (uid + 1) == 0 ? 1 : (uid + 1);
                Touch touch = input.touches[index];
                touch.uid = uid;
                touch.x = event.getX(action_index);
                touch.y = event.getY(action_index);
                touch.down = true;
                touch.up   = false;
            } break;

            case MotionEvent.ACTION_CANCEL: {
                input.reset();
            } break;

            case  MotionEvent.ACTION_UP:
            case  MotionEvent.ACTION_POINTER_UP: {
                if(index >= GameInput.MAX_TOUCHES) return true;
                Touch touch = input.touches[index];
                touch.uid = 0;
                touch.x = event.getX(action_index);
                touch.y = event.getY(action_index);
                touch.down = false;
                touch.up   = true;
            } break;

            case  MotionEvent.ACTION_MOVE: {
                int len = event.getPointerCount();
                for(int i = 0; i < len; ++i) {
                    int j = event.getPointerId(i);
                    if(j < GameInput.MAX_TOUCHES) {
                        input.touches[j].x = event.getX(i);
                        input.touches[j].y = event.getY(i);
                    }
                }
            } break;

        }

        return true;
    }
}

class GameRenderer implements GLSurfaceView.Renderer {

    boolean gameIsInit;
    MainActivity activity;

    public GameRenderer(MainActivity activity) {
        super();
        this.gameIsInit = false;
        this.activity = activity;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        activity.onGameResize(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        if(!this.gameIsInit) {
            activity.onGameInit();
            gameIsInit = true;
        }

        activity.onGameUpdate();
        activity.onGameRender();
    }
}