package nl.nhtv.vktut;

import android.app.NativeActivity;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;

/**
 * Created by Rick on 2-12-2016.
 */

public class WrappedNativeActivity extends NativeActivity {
    static {
        try {
            //System.loadLibrary("MGD");
            //android.util.Log.i("[ MGD ]", "Goddum!");
        } catch (UnsatisfiedLinkError e) {
            // Feel free to remove this log message.
            // Ok
            android.util.Log.i("[ MGD ]", "libMGD.so not loaded.");
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,WindowManager.LayoutParams.FLAG_FULLSCREEN);

        super.onCreate(savedInstanceState);
    }

    public void ShowPopup ( String text )
    {
        new android.app.AlertDialog.Builder( this )
                .setTitle ( "Error" )
                .setMessage( text )
                .setNeutralButton( android.R.string.ok, new DialogInterface.OnClickListener() {
                    public void onClick ( DialogInterface dialog, int which )
                    {

                    }
                })
                .setIcon(android.R.drawable.ic_dialog_alert)
        .show ( );
    }
}
