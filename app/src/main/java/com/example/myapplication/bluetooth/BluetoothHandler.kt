package com.example.myapplication.bluetooth

import android.content.Context
import com.example.dronevision.presentation.ui.bluetooth.BluetoothConnection

interface BluetoothHandler {
    fun setupBluetooth(
        context: Context,
        systemService: Any,
        listener: BluetoothActivityListener,
    ): BluetoothConnection
    fun sendMessage(message: String, id: Int)
}