package com.example.myapplication.utils


import android.Manifest
import android.app.Activity
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Build
import android.os.Environment
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.core.content.PermissionChecker

object PermissionTools {
    private val REQUIRED_PERMISSION_LIST = arrayOf(
        Manifest.permission.INTERNET,  // API requests
        Manifest.permission.BLUETOOTH,  // Bluetooth connected products
        Manifest.permission.BLUETOOTH_CONNECT,
        Manifest.permission.BLUETOOTH_SCAN,
        Manifest.permission.BLUETOOTH_PRIVILEGED,
        Manifest.permission.BLUETOOTH_ADMIN,  // Bluetooth connected products
    )

    private const val REQUEST_PERMISSION_CODE = 12345
    private val missingPermission: MutableList<String> = ArrayList()


    /**
     * Checks if there is any missing permissions, and
     * requests runtime permission if needed.
     */
    fun checkAndRequestPermissions(appCompatActivity: AppCompatActivity) {
        // Check for permissions
        for (permission in REQUIRED_PERMISSION_LIST) {
            if (ContextCompat.checkSelfPermission(
                    appCompatActivity,
                    permission
                ) != PackageManager.PERMISSION_GRANTED
            ) {
                missingPermission.add(permission)
            }
        }
        // Request for missing permissions
        if (missingPermission.isNotEmpty()) {
            ActivityCompat.requestPermissions(
                appCompatActivity,
                missingPermission.toTypedArray(),
                REQUEST_PERMISSION_CODE
            )
        }
    }
}