package com.example.dronevision.presentation.ui.bluetooth

import android.annotation.SuppressLint
import android.app.Dialog
import android.content.Context
import android.text.TextUtils
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.RecyclerView
import com.example.myapplication.R
import com.example.myapplication.bluetooth.BluetoothListItem
import com.example.myapplication.databinding.BluetoothListItemBinding

class BluetoothRecyclerViewAdapter(
    private val bluetoothCallback: BluetoothCallback,
    context: Context,
    layoutInflater: LayoutInflater
) :
    RecyclerView.Adapter<BluetoothRecyclerViewAdapter.ItemHolder>() {

    private var items = mutableListOf<BluetoothListItem>()
    private val mContext = context
    private val mLayoutInflater = layoutInflater

    fun setData(items: List<BluetoothListItem>) {
        val result = items.toMutableList()
        this.items = result
        notifyDataSetChanged()
    }

    class ItemHolder(view: View) : RecyclerView.ViewHolder(view) {

        private val binding = BluetoothListItemBinding.bind(view)

        fun setData(
            item: BluetoothListItem,
            bluetoothCallback: BluetoothCallback,
            context: Context,
            layoutInflater: LayoutInflater
        ) {
            binding.name.text = item.name
            binding.mac.text = item.mac

            binding.bluetoothGadget.setOnClickListener {
                    bluetoothCallback.onClick(BluetoothListItem(name = item.name, mac = item.mac))
            }
        }

        private fun checkInput(title: String): Boolean {
            return !(TextUtils.isEmpty(title))
        }

        companion object {
            fun create(parent: ViewGroup): ItemHolder {
                return ItemHolder(
                    LayoutInflater.from(parent.context)
                        .inflate(R.layout.bluetooth_list_item, parent, false)
                )
            }
        }
    }

    class ItemComparator : DiffUtil.ItemCallback<BluetoothListItem>() {
        override fun areItemsTheSame(
            oldItem: BluetoothListItem,
            newItem: BluetoothListItem
        ): Boolean {
            return oldItem.mac == newItem.mac
        }

        @SuppressLint("DiffUtilEquals")
        override fun areContentsTheSame(
            oldItem: BluetoothListItem,
            newItem: BluetoothListItem
        ): Boolean {
            return oldItem == newItem
        }

    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ItemHolder {
        return ItemHolder.create(parent)
    }

    override fun onBindViewHolder(holder: ItemHolder, position: Int) {
        items.getOrNull(position)?.let {
            holder.setData(
                item = it,
                bluetoothCallback = bluetoothCallback,
                context = mContext,
                layoutInflater = mLayoutInflater
            )
        }
    }

    override fun getItemCount(): Int {
        return items.size
    }
}
