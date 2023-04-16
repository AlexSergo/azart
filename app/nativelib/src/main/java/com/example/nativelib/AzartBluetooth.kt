package com.example.nativelib

class AzartBluetooth {
    companion object {
        init {
            System.loadLibrary("azartBluetooth")
        }
    }

    external fun readBytes(): ByteArray
    external fun writeBytes(byteArray: ByteArray)
    external fun writeString(byteArray: ByteArray)
    external fun readStringAsBytes(): ByteArray
    external fun isSuccessfulStatus(): Boolean
}