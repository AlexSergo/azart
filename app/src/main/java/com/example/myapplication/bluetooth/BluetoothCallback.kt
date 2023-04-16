package com.example.dronevision.presentation.ui.bluetooth

import com.example.myapplication.bluetooth.BluetoothListItem

interface BluetoothCallback {
    fun onClick(item: BluetoothListItem)
}