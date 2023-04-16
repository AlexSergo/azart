package com.example.dronevision.presentation.ui.bluetooth

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import com.example.dronevision.presentation.ui.bluetooth.ConnectionBluetoothObject.connectionList
import com.example.myapplication.bluetooth.BluetoothActivityListener

class BluetoothConnection(
    private val adapter: BluetoothAdapter,
    private val listener: BluetoothActivityListener
) {

    lateinit var connectionThread: ConnectThread
    private var _isConnected: Boolean = false
    val isConnected: Boolean get() = _isConnected
    private var _isAzartConnected = false

    fun getAdapter(): BluetoothAdapter {
        return adapter
    }

    @SuppressLint("MissingPermission")
    fun connect(mac: String) {
        val uuid = "00001101-0000-1000-8000-00805F9B34FB"
        if (!adapter.isEnabled || mac.isEmpty())
            return
        val device = adapter.getRemoteDevice(mac)
        device.let {
            connectionThread = ConnectThread(it, uuid, listener, ::setAzartConnected)
            connectionThread.start()
        }
        connectionList[1] = connectionThread
        _isConnected = true
    }

    fun setAzartConnected(id: Int, isConnected: Boolean){
        if (id == 1)
            _isAzartConnected = isConnected
    }

    fun sendMessage(byteArray: ByteArray, id: Int = 1 ){
        if (_isAzartConnected)
            try {
                connectionList.getValue(id).receiveThread.sendMessage(byteArray)
            }catch (_ :Exception){
                listener.showMessage("Нет Bluetooth соединения!")
            }
    }

    fun sendMessage(message: String, id: Int) {
        if (_isConnected)
            connectionList.getValue(id).receiveThread.sendMessage(message.toByteArray())
        else{
            connectionList.remove(id)
            listener.showMessage("Невозможно отправить!")
        }
    }
}
