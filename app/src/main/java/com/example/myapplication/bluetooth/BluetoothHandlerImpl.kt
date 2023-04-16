package com.example.myapplication.bluetooth


import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothServerSocket
import android.bluetooth.BluetoothSocket
import android.content.Context
import com.example.dronevision.presentation.ui.bluetooth.BluetoothConnection
import com.example.dronevision.presentation.ui.bluetooth.BluetoothReceiver
import java.io.IOException
import java.util.*


class BluetoothHandlerImpl : BluetoothHandler {

    private var bluetoothAdapter: BluetoothAdapter? = null
    private lateinit var bluetoothConnection: BluetoothConnection
    private lateinit var receiver: BluetoothReceiver
    private lateinit var listener: BluetoothActivityListener
    private var socket: BluetoothSocket? = null
//    val uuid = "00001101-0000-1000-8000-00805F9B34FB"

    override fun setupBluetooth(
        context: Context, systemService: Any, listener: BluetoothActivityListener
    ): BluetoothConnection {

        val bluetoothManager = systemService as BluetoothManager
        bluetoothAdapter = bluetoothManager.adapter

        if (bluetoothAdapter != null) {
            bluetoothConnection = BluetoothConnection(bluetoothAdapter!!, listener)
        }
        this.listener = listener
        return bluetoothConnection
    }

    override fun sendMessage(message: String, id: Int) {
        if (bluetoothConnection.isConnected)
            bluetoothConnection.sendMessage(message, id)
        else
            socket?.let {
                receiver.sendMessage(message.toByteArray())
            }
    }
}
