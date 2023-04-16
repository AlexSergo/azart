#include <jni.h>
#include <string>
#include "../../../../../../DroneVision/app/src/main/cpp/include/azart_proto_cl.h"
#include <memory>
#include <queue>

void mprog();

static std::shared_ptr<azart_cl> azartp{nullptr};
static std::thread az_workout(mprog);
static std::queue<char> rx_q;
static std::queue<char> tx_q;

int rx_f(char *b){
    if(rx_q.empty()) return 0;
    *b = rx_q.front();
    rx_q.pop();
    return 1;
}

int tx_f(char b){
    tx_q.push(b);
    return 1;
}

int tx_fv(std::vector<char> data){
    for(auto el : data){
        tx_q.push(el);
    }
    return 1;
}

void mprog(){
    if (azartp != nullptr) return;
    azartp = std::make_shared<azart_cl>();
    azartp->set_rx_func(rx_f);
    azartp->set_txm_func(tx_fv);
    auto ctp = std::chrono::system_clock::now()+ std::chrono::milliseconds {10};

    while(true){
        azartp->poll();
        std::this_thread::sleep_until(ctp);
        ctp = std::chrono::system_clock::now() + std::chrono::microseconds {100};    }


};


extern "C"
JNIEXPORT void JNICALL
Java_com_example_nativelib_AzartBluetooth_writeBytes(JNIEnv *env, jobject thiz, jbyteArray data) {

    // Get elements of the array
    jbyte *elements = env->GetByteArrayElements(data, 0);

    // Get length of the array
    int len = env->GetArrayLength(data);

    // Loop through the array 'Count' times, assigning 'Value' to each
    // element.
    for (int i = 0; i < len; i++)
        rx_q.push(elements[i]);
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_example_nativelib_AzartBluetooth_readBytes(JNIEnv *env, jobject thiz) {
int size = tx_q.size() <= 512 ? tx_q.size() : 512;
jbyte a[512] = {};
int i = 0;
while (i < size) {
jbyte b = tx_q.front();
tx_q.pop();
a[i++] = b;
}
jbyteArray ret = env->NewByteArray(size);
env->SetByteArrayRegion (ret, 0, size, a);
return ret;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_nativelib_AzartBluetooth_writeString(JNIEnv *env, jobject thiz, jbyteArray data) {
//    const char *nativeString = (char*)env->GetByteArrayElements(data, 0);
//    azartp->send_sds(nativeString);

    jint length = (*env).GetArrayLength(data);
    const char *nativeString = (char*)env->GetByteArrayElements(data, 0);
    char *buff;
    buff = (char*)malloc((length + 1) * sizeof(char));
    memcpy(buff,nativeString, length*sizeof(char ));
    buff[length] = '\0';
    azartp->send_sds(buff);
    free(buff);
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_example_nativelib_AzartBluetooth_readStringAsBytes(JNIEnv *env, jobject thiz) {
    auto sds = azartp->get_sds();
    if(sds.empty())  return nullptr;

    jbyteArray ret2 = env->NewByteArray(sds.size());
    env->SetByteArrayRegion (ret2, 0, sds.size(), reinterpret_cast<const jbyte *>(sds.data()));

    return ret2;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_nativelib_AzartBluetooth_isSuccessfulStatus(JNIEnv *env, jobject thiz) {
    return azartp->get_send_status() == 1;
}
