package com.example.opengles;

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.example.opengles.databinding.ActivityMainBinding


class MainActivity : AppCompatActivity() {
    private lateinit var mView : GLESView
    private lateinit var binding : ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        mView = GLESView(application)
        setContentView(mView)
    }

    override fun onPause() {
        super.onPause()
        mView.onPause()
    }

    override fun onResume() {
        super.onResume()
        mView.onResume()
    }
}