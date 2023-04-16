package com.example.dronevision.presentation.ui.bluetooth

import android.bluetooth.BluetoothSocket
import com.example.myapplication.bluetooth.BluetoothActivityListener
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream


class BluetoothReceiver(
    private val bluetoothSocket: BluetoothSocket,
    private val listener: BluetoothActivityListener,
    private val connected: (isConnected: Boolean) -> Unit
) : Thread() {

    private val BUFFER_SIZE = 512
    private var inputStream: InputStream? = null
    private var outputStream: OutputStream? = null

    init {
        try {
            inputStream = bluetoothSocket.inputStream
            outputStream = bluetoothSocket.outputStream
        } catch (_: IOException) {

        }
    }

    override fun run() {
        val buffer = ByteArray(BUFFER_SIZE)
        var message = ""
        while (true) {
            try {
                val size = inputStream?.read(buffer)
                message += String(buffer, 0, size!!)
                listener.receiveBytes(buffer)
            } catch (e: IOException) {
                listener.showMessage("Ошибка, сбой соединения!")
                connected(false)
                return
            }
        }
    }

    fun sendMessage(byteArray: ByteArray) {
        try {
            outputStream?.write(byteArray)
        } catch (_: IOException) {
            listener.showMessage("Не удалось отправить!")
            connected(false)
        }
    }
}
