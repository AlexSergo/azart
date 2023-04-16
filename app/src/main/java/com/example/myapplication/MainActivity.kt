package com.example.myapplication

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Toast
import com.example.dronevision.presentation.ui.bluetooth.BluetoothCallback
import com.example.dronevision.presentation.ui.bluetooth.BluetoothConnection
import com.example.myapplication.bluetooth.BluetoothActivityListener
import com.example.myapplication.bluetooth.BluetoothHandler
import com.example.myapplication.bluetooth.BluetoothHandlerImpl
import com.example.myapplication.bluetooth.BluetoothListItem
import com.example.myapplication.databinding.ActivityMainBinding
import com.example.myapplication.utils.PermissionTools
import com.example.nativelib.AzartBluetooth
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking

class MainActivity : AppCompatActivity(), BluetoothActivityListener,
    BluetoothHandler by BluetoothHandlerImpl() {

    private lateinit var binding: ActivityMainBinding
    private val azart = AzartBluetooth()
    private var connection: BluetoothConnection? = null
    private var dialog: SelectBluetoothFragment? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        PermissionTools.checkAndRequestPermissions(this)

        connection = setupBluetooth(
            context = this,
            systemService = getSystemService(BLUETOOTH_SERVICE),
            listener = this
        )
        dialog = SelectBluetoothFragment(connection!!.getAdapter(), object : BluetoothCallback {
            override fun onClick(item: BluetoothListItem) {
                item.let {
                    connection!!.connect(it.mac)
                }
                dialog?.dismiss()
            }
        })

        binding.connectionButton.setOnClickListener {
            dialog?.show(supportFragmentManager, "ActionBottomDialog")
        }
        binding.button.setOnClickListener {
            sendData("Привет".toByteArray())
        }

        startNative()
    }

    private fun startNative() {
        val isRunning = true
        GlobalScope.launch(Dispatchers.IO){
            while (isRunning) {
                val controlData: ByteArray =
                    azart.readBytes()// считывает данные для отправки на р/cт
                if (controlData.isNotEmpty()) {
                    writeControl(controlData)
                }
                try {
                    Thread.sleep(1)
                } catch (e: InterruptedException) {
                    throw java.lang.RuntimeException(e)
                }
            }
        }

        GlobalScope.launch(Dispatchers.IO){
            while (isRunning) {
                val bytes: ByteArray? =
                    azart.readStringAsBytes()
                if (bytes != null && bytes.isNotEmpty()) {
                    manageReceivedData(bytes);
                    // здесь мы получаем данные SDS от р/ст
                }
                try {
                    Thread.sleep(100)
                } catch (e: InterruptedException) {
                    throw java.lang.RuntimeException(e)
                }
            }
        }
    }

    override fun showMessage(str: String) {
        runOnUiThread {
            Toast.makeText(applicationContext, str, Toast.LENGTH_SHORT).show()
        }
    }

    override fun receiveBytes(bytes: ByteArray) {
        azart.writeBytes(bytes) // отправляем данные, полученные от bluetooth в native-lib
    }

    private fun manageReceivedData(bytes: ByteArray) {
        val message = String(bytes)
        showMessage(message)
    }

    private fun writeControl(data: ByteArray) {
        // отправка данных в bluetooth или usb
        connection?.sendMessage(data)
    }

     fun sendData(data: ByteArray) {
        // В отдельном потоке (функция блокирующая, чтобы не подвешивать GUI) отправляем данные в native-lib (writeNative)
         runBlocking {
             launch{
                 if (data.isNotEmpty())
                     azart.writeString(data)
             }
         }
        // а в постоянном цикле ведется опрос native-lib (readNativeControl) и, если готовы данные, они будут переданы далее (на usb/bluetooth) через writeControl
    }
}