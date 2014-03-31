package com.example.pebble2048;

import java.util.UUID;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v7.app.ActionBarActivity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;

public class MainActivity extends ActionBarActivity {

	
	
	
	public void updateScore(View view) {
		TextView scoreZone = (TextView) findViewById(R.id.textView1);
		String myText = scoreZone.getText().toString();
	    if (Integer.parseInt(myText) < 2048) {
	    	scoreZone.setText("" + 2*Integer.parseInt(myText));
	    } else if (Integer.parseInt(myText) == 2048){
	    	scoreZone.setText("gg.");
	    }
	}
	
	public void startGame(View view) {
		UUID PEBBLE_APP_UUID = UUID.fromString("2becaf0a-9b51-4bd9-9070-f834db2fbaad");
	    PebbleKit.startAppOnPebble(getApplicationContext(), PEBBLE_APP_UUID);
	}
	
	public void endGame(View view) {
		UUID PEBBLE_APP_UUID = UUID.fromString("2becaf0a-9b51-4bd9-9070-f834db2fbaad");
	    PebbleKit.closeAppOnPebble(getApplicationContext(), PEBBLE_APP_UUID);
	}
	
	public void send(View view) {
		UUID PEBBLE_APP_UUID = UUID.fromString("2becaf0a-9b51-4bd9-9070-f834db2fbaad");
		PebbleDictionary data = new PebbleDictionary();
		data.addUint8(0, (byte) 42);
		data.addString(1, "a string");
		PebbleKit.sendDataToPebble(getApplicationContext(), PEBBLE_APP_UUID, data);
	}

	
	
	
	
	
	
	
	
	
	
	
	
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		if (savedInstanceState == null) {
			getSupportFragmentManager().beginTransaction()
					.add(R.id.container, new PlaceholderFragment()).commit();
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {

		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	/**
	 * A placeholder fragment containing a simple view.
	 */
	public static class PlaceholderFragment extends Fragment {

		public PlaceholderFragment() {
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {
			View rootView = inflater.inflate(R.layout.fragment_main, container,
					false);
			return rootView;
		}
	}

}
