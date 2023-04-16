package com.example.dronevision.presentation.ui.bluetooth

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothSocket
import com.example.myapplication.bluetooth.BluetoothActivityListener
import java.io.IOException
import java.util.*


@SuppressLint("MissingPermission")
class ConnectThread(
    device: BluetoothDevice,
    private val uuid: String,
    private val listener: BluetoothActivityListener,
    private val connected: (id: Int, isConnected: Boolean) -> Unit
) : Thread() {

    private var socket: BluetoothSocket? = null
    lateinit var receiveThread: BluetoothReceiver

    init {
        try {
            socket = device.createInsecureRfcommSocketToServiceRecord(UUID.fromString(uuid))
        } catch (_: IOException) { }
    }

    override fun run() {
        try {
            listener.showMessage("Подключение...")
            socket?.connect()
            receiveThread = BluetoothReceiver(socket!!, listener, ::isConnected)
            receiveThread.start()
            listener.showMessage("Подключено!")
            connected(1, true)
        } catch (e: IOException) {
            listener.showMessage("Невозможно подключиться!")
            connected(1, false)
            closeConnection()
        }
    }

    fun isConnected(isConnected: Boolean) {
        connected(1, isConnected)
        if (!isConnected)
            listener.showMessage("Невозможно подключиться!")
    }

    private fun closeConnection() {
        try {
            socket?.close()
        } catch (_: IOException) {

        }
    }
}
