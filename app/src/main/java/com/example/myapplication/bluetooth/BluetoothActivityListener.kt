package com.example.myapplication.bluetooth

interface BluetoothActivityListener {
    fun showMessage(str: String)
    fun receiveBytes(bytes: ByteArray)
}
