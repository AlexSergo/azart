package com.example.myapplication

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.DialogFragment
import androidx.recyclerview.widget.GridLayoutManager
import com.example.dronevision.presentation.ui.bluetooth.BluetoothCallback
import com.example.dronevision.presentation.ui.bluetooth.BluetoothRecyclerViewAdapter
import com.example.myapplication.bluetooth.BluetoothListItem
import com.example.myapplication.databinding.FragmentSelectBluetoothBinding

class SelectBluetoothFragment(
    private var bluetoothAdapter: BluetoothAdapter?,
    private val bluetoothCallback: BluetoothCallback
) : DialogFragment() {

    private lateinit var binding: FragmentSelectBluetoothBinding
    private lateinit var recyclerViewAdapter: BluetoothRecyclerViewAdapter

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        binding = FragmentSelectBluetoothBinding.inflate(layoutInflater)
        binding.recyclerView.layoutManager = GridLayoutManager(requireContext(), 1)
        recyclerViewAdapter = BluetoothRecyclerViewAdapter(bluetoothCallback, requireContext(), layoutInflater)
        binding.recyclerView.adapter = recyclerViewAdapter
        getPairedDevices()
        return binding.root
    }

    @SuppressLint("MissingPermission")
    private fun getPairedDevices() {
        val pairedDevices: Set<BluetoothDevice>? = bluetoothAdapter?.bondedDevices
        val tempList = ArrayList<BluetoothListItem>()
        pairedDevices?.forEach {
            tempList.add(BluetoothListItem(name = it.name, mac = it.address))
        }
        recyclerViewAdapter.setData(tempList)
    }
}
