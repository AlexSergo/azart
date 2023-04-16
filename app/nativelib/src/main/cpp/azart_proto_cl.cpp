// Copyright (c) 2022 OOO FIRMA 'NOVOPLAN' . All rights reserved.
// Author: Vladimir Guzenko, <vg@mptsrv.com>

#include <string>
#include <iostream>
#include <chrono>
#include "cstdlib"
#include "cstdio"
#include "cstring"
#include "include/azart_proto_cl.h"
#include <cassert>
#include <algorithm>

#ifdef GW_DEBUG
#include "agw_debug.h"
#endif


uint8_t azart_cl::empty_frame[6] = {0x00, 0x01, 0x00, 0x00, 0xB3, 0xF0};
const uint16_t azart_cl::crc_table[256] =
    {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5,
        0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b,
        0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210,
        0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
        0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c,
        0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401,
        0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b,
        0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6,
        0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738,
        0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4, 0x58e5,
        0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969,
        0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96,
        0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc,
        0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
        0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03,
        0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd,
        0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6,
        0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
        0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a,
        0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb,
        0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1,
        0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c,
        0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2,
        0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb,
        0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
        0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447,
        0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,
        0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2,
        0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9,
        0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827,
        0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d, 0xdb5c,
        0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
        0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0,
        0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d,
        0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07,
        0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
        0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba,
        0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,
        0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
    };


void
azart_cl::process_byte(uint8_t b) {
    static int lf_nums = 0;
    static unsigned packet_length = 0;
    static bool debug = false;
    packet_error_time = system_clock::now() + packet_error_interval;


    if(current_work_state == INITIAL_AT_S2){
        if(b == 0xc0){
            set_work_state(work_states::INITIAL_S2);

        }else{
            raw_recived_frame[buff_pos++] = b;
            if(b == 0x0A) {
                lf_nums++;
            }
            if(lf_nums == 3){
                set_work_state(INITIAL_S1);
                set_process_state(IDLE);
                lf_nums = 0;
                buff_pos = 0;
            }
            return;
        }
    }

    switch (current_link_state) {
        case LINK_WAIT:
            if (b == 0xC0) {
                current_link_state = START_GOT;
                buff_pos = 0;
                packet_length = 0;
            }
            break;
        case DB_GOT:
            if (b == 0xDC) raw_recived_frame[buff_pos++] = 0xC0;
            if (b == 0xDD) raw_recived_frame[buff_pos++] = 0xDB;
            if (b != 0xDC && b != 0xDD) {
                current_link_state = LINK_WAIT;// вот это поворот ;)
                break;
            }

            current_link_state = START_GOT;
#ifndef PE_SLIP
            if(packet_length == 0 && buff_pos > 3){
                packet_length = ((unsigned )raw_recived_frame[2]) << 8;
                packet_length |= raw_recived_frame[3];
                packet_length += 6;
            }
            if (packet_length == buff_pos){
                current_link_state = END_GOT;
            }
#endif
            break;
        case START_GOT:
            if (b == 0xC0) {
                current_link_state = END_GOT;

                break;
            }
            if (b == 0xDB) {
                current_link_state = DB_GOT;

                break;
            }
#ifndef PE_SLIP

            if(buff_pos == 2)
                packet_length = ((unsigned )b) << 8;
            if(buff_pos == 3){
                packet_length |= b;
                if(packet_length == 0)
                    packet_length = 6; //пустой фрейм
                else
                    packet_length +=  6; // Полная длинна фрейма

            }
#endif
            raw_recived_frame[buff_pos++] = b;

#ifndef PE_SLIP
            if (packet_length == buff_pos){
                current_link_state = END_GOT;
            }
#endif
            break;
        case END_GOT:
            break;
        default:
            current_link_state = LINK_WAIT;
    }
    if (current_link_state == END_GOT) {
        //прием сообщения закончен
        if (crc_check()) {
            process_frame();
            link_error_time = system_clock::now() + link_error_interval;
        } else {
            frame_index--;
        }

        current_link_state = LINK_WAIT;
    }

}

void azart_cl::process_frame() {
/*
 *  #       Значение                    Размер байт
 *  0       индекс кадра                    1
 *  1       адрес устройства рс485          1
 *  2       длинна поля данных L ст байт    1
 *  3       длинна поля данных L мл байт    1
 *  4 - 4+L Данные кадра                    L(0-65535)
 *  +1      CRC ст байт                     1
 *  +2      CRC мл байт                     1
 *
 * */
    auto fh = (frame_header_s *) raw_recived_frame;
    uint16_t frame_length = ((uint16_t) fh->data_len_h) << 8;
    frame_length |= fh->data_len_l;

    if(modem_open){// на случай если связь с радиостанцией потеряна, а модем был открыт.
        if(modem_timeout_time < std::chrono::system_clock::now()){
            modem_open = false;
        }
    }

    if (frame_length == 0) {
#ifdef GW_DEBUG
        agw_debug("RX: EMPTY")
#endif
        current_rx_frame_type = EMPTY_FRAME;
        set_process_state(GOT_FRAME);
        return;
    }
#ifdef GW_DEBUG
    agw_to_hex(raw_recived_frame, frame_length+6, "RX: ");
#endif

    frame_data_header_s *fdh;
    uint16_t pos = sizeof(frame_header_s);
    uint16_t packet_length;
    uint8_t *tmp;
    static int sds_message_data = 0;
    // +CTTCT:1+CTSDSR:12,1033,0,3130,0,648204010574657374\r\n
    while (pos < frame_length) {

        fdh = (frame_data_header_s *) (raw_recived_frame + pos);
        packet_length = ((uint16_t) fdh->data_len_h) << 8;
        packet_length |= fdh->data_len_l;
        tmp = &raw_recived_frame[pos + sizeof(frame_data_header_s)];
        pos += sizeof(frame_data_header_s) + packet_length;// следующий!!!


        if (sds_message_data) {
            static int count = 4;
            sds_message_data = 0;
            memcpy(sds_iso_string, tmp, packet_length);
            sds_iso_string[packet_length] = '\0';
            if(--count == 0){// станция присылает 4 раза одно и тоже
                // TODO потенциальная ошибка. Если предположить что 4 раза это отправка через эфир в кол-ве 4 раз
                // а не просто дубляж на уровне протокола обмена
                sds_ready = 1;
                count = 4;
            }

            continue;
        }

        if(fdh->type == data_types::DATA_RP_KCC){
            //пакетик данных длинна 58
            constexpr int rp_header_len = 4;
            auto *rp_header = (rp_kcc_data_header_s*)tmp;
            unsigned rp_len = rp_header->data_len_l | (rp_header->data_len_h << 8);
            --rp_len; //из длинные 1 байт который ушёл в заголовок
            switch (modem_rate) {
                case data_rates::RATE_4800_LOW_PROTECT_INTERLEAVE_4:[[fallthrough]];
                case data_rates::RATE_4800_LOW_PROTECT_INTERLEAVE_8:{
                    --rp_len;
                    break;
                };
                case data_rates::RATE_2400_HI_PROTECT_INTERLEAVE_4:[[fallthrough]];
                case data_rates::RATE_2400_HI_PROTECT_INTERLEAVE_8:{
                    rp_len = 18;
                };
                default:
                    ;
            }
            modem_timeout_time = std::chrono::system_clock::now() + modem_timeout_interval;// обновим
            std::copy(tmp + rp_header_len, tmp + rp_header_len + rp_len, std::back_inserter(modem_rx_data));
        }


        if(strstr((char*)tmp,"OK") != nullptr) last_code = AT_OK;

        if(strstr((char*)tmp,"ERROR") != nullptr) last_code = AT_ERROR;

        if(strstr((char*)tmp,"CONNECT") != nullptr) {
            last_code = AT_CONNECT;
        }

        if(strstr((char*)tmp,"+CTICN") != nullptr) {
            //new modem call incoming
            utils_set_modem_param_on_invite((char*)tmp, packet_length);
        }

        if(strstr((char*)tmp,"+CTCR") != nullptr) {
            modem_open = false;// конец вызова
        }


        if(strstr((char*)tmp,"TXBUF") != nullptr) {
            //$TXBUF:0, 524288
            std::string ts;
            std::copy(tmp, tmp + packet_length, std::back_insert_iterator<std::string >(ts));
            auto p1  = ts.find(':');
            auto p2  = ts.find(',');

            if(p1 != std::string::npos && p2 != std::string::npos){
                modem_buffer_uzed = std::stoi(ts.substr(p1 + 1));
                modem_buffer_max = std::stoi(ts.substr(p2 + 1));
            }
        }

        if (memcmp(tmp, "+CMGS", 5) == 0) {//приходит по окончанию приёма сообщения
//            uint16_t pl = strlen((char*)sds_iso_string);

            //  (sds_iso_string,tmp,packet_length);
//          char *tmp2 = (char*)sds_iso_string;
//            while (pl){
//                buff_azart_to_bt.Write(*(tmp2));
//                pl--;
//                tmp2++;
//            }
//            buff_azart_to_bt.Write('\r');
//            buff_azart_to_bt.Write('\n');
        }

        if(sds_send_status == sds_send_states::SEND_INIT){
            /* При срыве, приходит такое
                                     E  R  R  O  R           O  K
                  C0E601000D0500 05 45 52 52 4F 52 05 00 02 4F 4B 26 B6 C0
             */

            if (memcmp(tmp, "ERROR", 5) == 0) {//Ошибка, скорее всего отправки
                sds_send_status = sds_send_states::SEND_FAIL;
            }
            if (memcmp(tmp, "+CMGS", 5) == 0) {
                sds_send_status = sds_send_states::SEND_OK;
            }
        }


        if(memcmp(tmp, "+CNUM",5) == 0){// собственный номер РС
            static char msg_h[32];
            static char *pmsg_h;
            int i = 0;
            memcpy(msg_h, tmp, packet_length);
            pmsg_h = strtok(msg_h, ",");
            while (pmsg_h != nullptr) {
                if (i == 1) {// мой номер
                    self_number = atoi(pmsg_h);
                }
                pmsg_h = strtok(nullptr, ",");
                i++;
            }
        }

        if (strstr((char*)tmp,"CTSDSR") != nullptr
            && sds_message_data == 0)
        {

            //+CTSDSR:12,249,0,2,0,200..28200010531B231C53B2D323B313636313338313330383B333$
            //заголовок сообщения, следующий блок текст сообщения
            // первые 8 символов сообщеия служебные. остальные по парно в кодировке iso-8859 в строковом представлении
            static char msg_h[64];
            static char *pmsg_h;
            int i = 0;
            memcpy(msg_h, tmp, packet_length);
            pmsg_h = strtok(msg_h, ",");
            //       0 =t , 1=from,2=x,3=to,4=x,5=len
            //+CTSDSR:12,1033,0,3130,0,64
            sds_message_data = 1;// следующий, наша нагрузка

            while (pmsg_h != nullptr) {
                if(i == 1){// from
                    memcpy(sds_from_num,pmsg_h, strlen(pmsg_h) + 1);
                }
                if (i == 5) {
                    sds_message_len = atoi(pmsg_h);
                    sds_message_len -= 32;
                    sds_message_len /= 8;
                }
                pmsg_h = strtok(nullptr, ",");
                i++;
            }
            //сообщение приходит 4 раза \:
        }

    }


    current_rx_frame_type = DATA_FRAME;
    set_process_state(GOT_FRAME);

}

void
azart_cl::utils_set_modem_param_on_invite(char *at_comm, size_t len)
{
    // +CTICN:46,0,7,0,2,1,1,0,4,0
    /*
     * +CTICN: <CC instance >, <call status>, <AI service>, [<calling party identity type>], [<calling party identity>],
        [<hook>], [<simplex>], [<end to end encryption>], [<comms type>], [<slots/codec>], [<called party identity type>],
        [<called party identity>], [<priority>]

        <AI service>    0 - TETRA speech;
                        1 - 7,2 kbit/s unprotected data;
                        2 - Low protection 4,8 kbit/s
                        int16_t interleaving depth = 1;
                        3 - Low protection 4,8 kbit/s
                        medium interleaving depth = 4;
                        4 - Low protection 4,8 kbit/s
                        long interleaving depth = 8;
                        5 - High protection 2,4 kbit/s
                        int16_t interleaving depth = 1;
                        6 - High protection 2,4 kbit/s
                        medium interleaving depth = 4;
                        7 - High protection 2,4 kbit/s
                        high interleaving depth = 8;
    */
    using namespace std;
    try {
        std::string ts;
        std::copy(at_comm, at_comm + len, std::back_insert_iterator<std::string>(ts));
        auto p1 = ts.find(":");
        ts = ts.substr(p1 + 1);    //skip AT
        ts = ts.substr(ts.find(",") + 1);// skip instance
        ts = ts.substr(ts.find(",") + 1);// skip status
        unsigned ai_service = stoi(ts);
        ts = ts.substr(ts.find(",") + 1);// skip service
        ts = ts.substr(ts.find(",") + 1);// skip ident type
        modem_caller_id = stoi(ts);
        switch (ai_service) {
            case 1: {
                modem_frame_size = SRATE_7200;
                break;
            }
            case 7:
                [[fallthrough]];
            case 6:
                [[fallthrough]];
            case 5: {
                modem_frame_size = SRATE_2400;
                break;
            }
            case 4:
                [[fallthrough]];
            case 3:
                [[fallthrough]];
            case 2: {
                modem_frame_size = SRATE_4800;
                break;
            }
            default:
                modem_frame_size = SRATE_7200;
        }
        modem_rate = static_cast<data_rates>(ai_service);
        modem_open = true;
        modem_timeout_time = std::chrono::system_clock::now() + modem_timeout_interval;
    }catch (std::logic_error &e){
        std::string err;
        err = e.what();
        err += " incoming data: ";
        std::copy(at_comm, at_comm + len, std::back_insert_iterator<std::string>(err));

        throw std::logic_error(err);
    }

}

inline void
azart_cl::initial_setings()
{

#ifdef RFCOMM
    static const char *at_cmd = "AT+CNUM?\r\n";
#else
    static const char *at_cmd = "AT+CNUM?\r\n";
#endif
    static const char *at_cmd_get_self = "AT+CNUM?\r\n";

    switch (current_work_state) {
        case INITIAL_S1:
            set_process_state(SEND_EMPTY_FRAME);
            set_work_state(INITIAL_S2);
            break;
        case INITIAL_S2:
            //TODO send_sds ATS23=921600
            if (current_rx_frame_type != EMPTY_FRAME) {// не получен пустой фрейм в ответ
                set_work_state(INITIAL_AT_S1);
                set_process_state(IDLE);
                break;
            }

            prepare_tx_frame(AT_COMM, at_cmd, strlen(at_cmd));

            set_process_state(SEND_DATA_FRAME);
            set_work_state(INITIAL_S3);
            break;
        case INITIAL_S3:
//            if(current_rx_frame_type != EMPTY_FRAME){// не получен пустой фрейм в ответ
//                set_work_state(INITIAL_S1);
//                set_process_state(IDLE);
//                break;
//            }
//            uart3_set_boud(19200U);

            set_work_state(ON_LINK);
            prepare_tx_frame(AT_COMM,at_cmd_get_self , strlen(at_cmd_get_self));
            set_process_state(SEND_DATA_FRAME);

            break;
        case ON_LINK:
//            radio_port_change_baud(230400U);
            if (radio_poll_time <= system_clock::now()) {
                set_process_state(SEND_EMPTY_FRAME);
            }
            break;
        case SEND_SDS_S1:
            set_process_state(SEND_DATA_FRAME);
            set_work_state(SEND_SDS_S2);
            break;
        case SEND_SDS_S2:
            prepare_tx_frame(data_types::AT_COMM, sds_string, strlen((char *) sds_string));
            set_process_state(SEND_DATA_FRAME);
            set_work_state(ON_LINK);
            break;
        case ERROR:

            set_work_state(INITIAL_AT_S1);
            set_process_state(SEND_EMPTY_FRAME);
            frame_index = 0;
            link_error_time = system_clock::now() + link_error_interval;
            break;
        case INITIAL_AT_S1:
            packet_error_time = system_clock::now() + packet_error_interval;

            send_raw_at();// Послать команду переключения в пакетный режим, сработает только через USB
//            send_empty_frame();
            link_error_time = system_clock::now() + link_error_interval;
            set_work_state(INITIAL_AT_S2);
            set_process_state(WAIT_FRAME);
//            set_process_state(SEND_EMPTY_FRAME);
            break;
        case INITIAL_AT_S2:
//            set_process_state(process_states::IDLE);
            set_work_state(INITIAL_S1);
            break;
    }

}

void azart_cl::main_loop()
{
    //Обработка входящего буффера
    char b;
    static int rx_ms_counter = 0;

    while (rx_f(&b)) {
        process_byte(b);
        rx_ms_counter = 0;
    }
#ifndef PE_SLIP

#endif
    //TODO time count;

    switch (current_process_state) {
        case IDLE:
            initial_setings();
            break;
        case WAIT_FRAME:

            if(modem_open){
                radio_poll_time = system_clock::now() + short_poll_interval;
            } else {
                radio_poll_time = system_clock::now() + poll_interval;
            }

            if(packet_error_time < system_clock::now()){
                set_process_state(RETRANSMIT_FRAME);
                packet_error_time = system_clock::now() + packet_error_interval;
            }

            break;
        case GOT_FRAME:
            link_error_time = system_clock::now() + link_error_interval;

            if(modem_open){
                radio_poll_time = system_clock::now() + short_poll_interval;
            } else {
                radio_poll_time = system_clock::now() + poll_interval;
            }

            if (current_rx_frame_type == AT_WAIT_R) {
                set_process_state(SEND_DATA_FRAME);
                break;
            }
            set_process_state(IDLE);

            break;
        case SEND_EMPTY_FRAME:
            if(radio_poll_time <= system_clock::now()){
                send_empty_frame();

                set_process_state(WAIT_FRAME);
                current_rx_frame_type = frame_types::INIT_STATE;

                if(modem_open){
                    radio_poll_time = system_clock::now() + short_poll_interval;
                } else {
                    radio_poll_time = system_clock::now() + poll_interval;
                }

            }
            break;
        case SEND_DATA_FRAME:
            if(radio_poll_time <= system_clock::now()) {
                set_process_state(WAIT_FRAME);

                send_data_frame();

                current_rx_frame_type = frame_types::INIT_STATE;
                if (modem_open) {
                    radio_poll_time = system_clock::now() + short_poll_interval;
                } else {
                    radio_poll_time = system_clock::now() + poll_interval;
                }
            }
            break;
        case RETRANSMIT_FRAME:
            frame_index = last_frame_index;
            if(last_frame_type == DATA_FRAME){
                set_process_state(SEND_DATA_FRAME);

            } else if(last_frame_type == EMPTY_FRAME){
                set_process_state(SEND_EMPTY_FRAME);
            }


            break;
    }
}

void
azart_cl::prepare_tx_frame(azart_cl::data_types frame_data_type, const void *frame_data, size_t data_len) {
    /*
            RADIO_DATA  = 3,
            AT_COMM     = 5,
            DATA_RP_KCC = 6,
            DATA_KCC_RP = 7

           #       Значение                    Размер байт
           0       начало кадра                    1        оформляется при отправке
           1       индекс кадра                    1
           2       адрес устройства рс485          1
           3       длинна поля данных L ст байт    1
           4       длинна поля данных L мл байт    1
           5 - 5+L Данные кадра                    L(0-65535)
           +1      CRC ст байт                     1
           +2      CRC мл байт                     1
           +3      конец кадра                          оформляется при отправке


           Данные кадра
           0        Тип данных                      1
           1        ст. байт - длина поля L         1
           2        мл. байт - длина поля L         1
           3‑(3+L)  данные                         (0-65535)

     */
#define MSG_ADD_LEN 3 // длинна данных кадра на 3 больше самих данных

    if (frame_data_type == AT_COMM) {
        unsigned frame_length = data_len + MSG_ADD_LEN;
        tx_frame[0] = frame_index++;//TODO индекс декрементировать если нет подтверждения
        tx_frame[2] = (uint8_t) ((frame_length) >> 8);
        tx_frame[3] = (uint8_t) ((frame_length) & 0xff);
        tx_frame[4] = AT_COMM;
        tx_frame[5] = (uint8_t) (data_len >> 8);;
        tx_frame[6] = (uint8_t) (data_len & 0xff);
        size_t i = 0;
        for (; i < data_len; i++) {
            tx_frame[7 + i] = ((uint8_t *) frame_data)[i];
        }
        uint16_t crc = crc_calc(tx_frame, 7 + i, 0xFFFFU); // длинна всех данных кадра начиная с индекса с заголовком
        tx_frame[7 + i++] = (uint8_t) (crc >> 8);
        tx_frame[7 + i++] = (uint8_t) (crc & 0xff);
        tx_frame_len = 7 + i;
    }
    if(frame_data_type == DATA_KCC_RP){
        tx_frame[0] = frame_index++;//TODO индекс декрементировать если нет подтверждения
        tx_frame[2] = (uint8_t) ((data_len + MSG_ADD_LEN) >> 8);
        tx_frame[3] = (uint8_t) ((data_len + MSG_ADD_LEN) & 0xff);
        tx_frame[4] = DATA_KCC_RP;
        tx_frame[5] = (uint8_t) (data_len >> 8);;
        tx_frame[6] = (uint8_t) (data_len & 0xff);
        size_t i = 0;
        for (; i < data_len; i++) {
            tx_frame[7 + i] = ((uint8_t *) frame_data)[i];
        }
        uint16_t crc = crc_calc(tx_frame, 7 + i, 0xFFFFU); // длинна всех данных кадра начиная с индекса с заголовком
        tx_frame[7 + i++] = (uint8_t) (crc >> 8);
        tx_frame[7 + i++] = (uint8_t) (crc & 0xff);
        tx_frame_len = 7 + i;
    }
}

void azart_cl::send_data_frame() {
    write_to_dataport(tx_frame, tx_frame_len);
    last_frame_index = tx_frame[0];
    last_frame_type = DATA_FRAME;
}

void
azart_cl::send_empty_frame()
{
    empty_frame[0] = frame_index++;
    uint16_t crc = crc_calc(empty_frame, 4, 0xFFFFU); // длинна всех данных кадра начиная с индекса с заголовком
    empty_frame[4] = (uint8_t) (crc >> 8);
    empty_frame[5] = (uint8_t) (crc & 0xff);
    write_to_dataport(empty_frame, sizeof(empty_frame));

    last_frame_index = empty_frame[0];
    last_frame_type = EMPTY_FRAME;
}



void
azart_cl::poll()
{

    if(rx_f == nullptr || tx_fm == nullptr) {
        assert(0);
    }
    using namespace std;

    main_loop();
    if (link_error_time <= system_clock::now()) {
        set_work_state(ERROR);
        set_process_state(IDLE);
        frame_index = 0;
        link_error_time = system_clock::now() + link_error_interval;
    }
}

int
azart_cl::crc_check()
{
    if(buff_pos == 0) return 0;
    if(buff_pos < 5) return 0;
    uint16_t crc_gen = crc_calc(raw_recived_frame, buff_pos - 2, 0xFFFFU);
    if (crc_gen == ((uint16_t) raw_recived_frame[buff_pos - 1] | (uint16_t) (raw_recived_frame[buff_pos - 2] << 8))) {
        return 1;
    }
    return 0;
}

inline void
azart_cl::write_to_dataport(uint8_t *d, size_t len)
{
#ifdef GW_DEBUG
//    std::vector<uint8_t> vtmp;
//    std::copy(d, d + len, std::back_inserter(vtmp));
//    agw_to_hex(vtmp.data(), vtmp.size(), "TX: ");
#endif
    std::vector<char> stmp;

    stmp.push_back(START_BYTE);

    while (len--) {
        if (*d != 0xC0 && *d != 0xDB) {
            stmp.push_back(*d);
        }
        if (*d == 0xDB) {
            stmp.push_back(0xDB);
            stmp.push_back(0xDD);
        }
        if (*d == 0xC0) {
            stmp.push_back(0xDB);
            stmp.push_back(0xDC);
        }

        d++;
    }

#ifdef PE_SLIP
    stmp.push_back(STOP_BYTE);
#endif
#ifdef GW_DEBUG
    agw_to_hex(stmp.data(),stmp.size(),"TX");
#endif
    tx_fm(stmp);

}


int
azart_cl::send_sds(char const  *string)
{
    // +1033;1В1Х;2;1;16;6183592;7412633;200;100;150;90;1\r\n
    if (string == nullptr) return 0;
    if (string[0] != '+') return 0;

    static const char *sds_prepare = "AT+CTSDS=12,0\r\n";
    while (current_process_state != IDLE){
        std::this_thread::sleep_for(std::chrono::milliseconds{1});
    }
    sds_send_status = SEND_INIT;
    prepare_tx_frame(data_types::AT_COMM, sds_prepare, strlen(sds_prepare));
    set_work_state(SEND_SDS_S1);

    int c = 0;
    size_t msg_length = 0;
    char radio_addr[8];
    char *tmp = (char*)string;// remove const

    tmp++;// skip '+' symbol
    //вычленим адрес

    while (*(tmp) != ';' && *(tmp) != '\0') {
        radio_addr[c++] = *tmp;
        tmp++;
    }
    radio_addr[c] = '\0';

    tmp++;// skip ';'

    msg_length = strlen(tmp) - 2; // remove r n
    msg_length = (msg_length * 8) + 32;
    static char  sds_iso_string_[512] = {'\0'};
    msm_decode(sds_iso_string_, tmp);

    snprintf((char *) sds_string, 512,
             "AT+CMGS=%s,%lu\r\n82000105%s",
             radio_addr,
             msg_length,
             sds_iso_string_); // -2 возможно без rn


    // AT+CMGS=<target>,<size>\r82000105<msg>
    //<size> - размер преобразованного текста сообщения, умноженный на 8, плюс 32, <msg> - текст сообщения.
//      tmp = (char *)sds_string;
//    while(*(tmp) != '\0'){
//        buff_azart_to_bt.Write(*(tmp));
//        tmp++;
//    }
    return 1;
}


void
azart_cl::msm_decode(char *dst, char *msg)
{
    static char conv[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    int p = 0;
    // TODO очень опасная функция. Нужно вычислить максимальный размер буфера в процессе отладки
    while (*(msg) != '\r' && *(msg) != '\0') {
        uint8_t c = *msg;
        p = sprintf(dst, "%c%c", conv[c >> 4], conv[c & 0xF]);
        dst += p;
        msg++;
    }
    sprintf(dst, "%c\r\n", 0x1A);
}

std::vector<char>
azart_cl::get_sds()
{
    std::vector<char>  ret;
    if (sds_ready == 0) return ret;

    msm_encode((char *) sds_string, (char *) sds_iso_string);
    std::copy(sds_string,sds_string + strlen((char*)sds_string),std::back_inserter(ret));
    sds_ready = 0;
    return ret;
}


void
azart_cl::msm_encode(char *dst, char *msg)
{
    msg += 8; //пропуск служебных данных
    uint8_t temp;
    memcpy(dst,sds_from_num, strlen((char*)sds_from_num));
    dst += strlen((char*)sds_from_num);
    *dst = ';';
    dst++;
    //TODO add num from
    while (*msg != '\0' && *msg != '\r') {
//        if > 0x39 to -68 else -48
        if ((uint8_t) (*msg) > 0x39)
            temp = ((*msg) - 55) << 4;
        else
            temp = ((*msg) - 48) << 4;
        msg++;
        if ((uint8_t) (*msg) > 0x39)
            temp |= ((*msg) - 55);
        else
            temp |= ((*msg) - 48);
        msg++;
        *dst = temp;
        dst++;
    }
    *dst = '\r';
    dst++;
    *dst = '\n';
    dst++;
    *dst = '\0';

}

void
azart_cl::set_rx_func(int(*rx)(char*))
{
    rx_f = rx;
}


void
azart_cl::set_txm_func(int(*tx)(std::vector<char>))
{
    tx_fm = tx;
}

void
azart_cl::radio_port_change_baud(uint32_t baud)
{
    //do nothing
    ;
}

void
azart_cl::send_raw_at()
{
    static const char *at_cmd = "AT$SERIALMODE=1\r\n";
    std::vector<char> tmp{at_cmd,at_cmd + strlen(at_cmd)};
    tx_fm(tmp);

}

int
azart_cl::get_send_status()
{
    using namespace std;
    auto break_tp = chrono::system_clock::now() + chrono::milliseconds (1500);
    while(sds_send_status == sds_send_states::SEND_INIT){
        this_thread::sleep_for(chrono::milliseconds{1} );
        if(break_tp < chrono::system_clock::now()) {
            sds_send_status = sds_send_states::SEND_FAIL;
        }
    }
    return sds_send_status;
}

bool
azart_cl::init_data_call(azart_cl::data_rates rate, int dest_radio_num, bool simplex )
{
    using namespace std;
    bool ret = false;
    const string call_clear = "ATS24=0\r\n";
    modem_rate = rate;
    if(rate == RATE_7200_UNPROTECTED) modem_frame_size = SRATE_7200;
    if(rate == RATE_4800_LOW_PROTECT_INTERLEAVE_4 ||
        rate == RATE_4800_LOW_PROTECT_INTERLEAVE_1 ||
        rate == RATE_4800_LOW_PROTECT_INTERLEAVE_8) modem_frame_size = SRATE_4800;
    if(rate >= RATE_2400_HI_PROTECT_INTERLEAVE_1) modem_frame_size = SRATE_2400;

    string set_mode("AT+CTSDC=" + to_string(rate) + ",0,,1," + to_string(simplex) + ",,,0\r\n");
    string dial_to("ATD" + to_string(dest_radio_num) + "\r\n");
//modem_open
    auto timeout = chrono::system_clock::now() + 2000ms;
    while (current_process_state != IDLE){
        this_thread::sleep_for(std::chrono::milliseconds{1});
        if(timeout < chrono::system_clock::now()) break;
    }

    prepare_tx_frame(data_types::AT_COMM, call_clear.c_str(), call_clear.length() );

    set_process_state(SEND_DATA_FRAME);
    last_code = AT_WAIT;

    while (last_code == AT_WAIT){
        this_thread::sleep_for(chrono::milliseconds{1});
        if(timeout < chrono::system_clock::now()) break;
    }

    if(last_code == AT_OK){
        prepare_tx_frame(data_types::AT_COMM, set_mode.c_str(), set_mode.length() );
        last_code = AT_WAIT;
        set_process_state(SEND_DATA_FRAME);

        while (last_code == AT_WAIT){
            this_thread::sleep_for(chrono::milliseconds{1});
            if(timeout < chrono::system_clock::now()) break;
        }

    }


    if(last_code == AT_OK){
        prepare_tx_frame(data_types::AT_COMM, dial_to.c_str(), dial_to.length() );
        last_code = AT_WAIT;
        set_process_state(SEND_DATA_FRAME);

        while (last_code == AT_WAIT){
            this_thread::sleep_for(chrono::milliseconds{1});
            if(timeout < chrono::system_clock::now()) break;
        }
    }

    if(last_code == AT_CONNECT){
        modem_open = ret = true;
    }

    return ret;
}

bool
azart_cl::send_binary(std::vector<uint8_t> data)
{
    using namespace std;
    bool ret{false};
    if(!modem_open){
        cerr << "modem closed" <<endl;
        return ret;
    }
    // размер буфера modem_buffer_uzed modem_buffer_max
    auto timeout = chrono::system_clock::now() + 5000ms;
    vector<uint8_t> buf;
    int frame_size = modem_frame_size;
    while(!data.empty()){
        buf.clear();
        buf.push_back(0x08); //первый байт указывает на количество бит в последнем байте ПАКД.46413....


        if(data.size() >= frame_size)
        {
            copy(data.begin(),
                 data.begin() + frame_size,
                 back_insert_iterator<vector<uint8_t>>(buf));
            data.erase(data.begin(),
                       data.begin() + frame_size);
        }
        else
        {
            copy(data.begin(),
                 data.end(),
                 back_insert_iterator<vector<uint8_t>>(buf));
            data.clear();
        }


        while (current_process_state != process_states::IDLE){
            this_thread::sleep_for(chrono::microseconds {100});
            if(timeout < chrono::system_clock::now()) break;

        }
        prepare_tx_frame(data_types::DATA_KCC_RP, buf.data(), buf.size());

        set_process_state(SEND_DATA_FRAME);
        modem_timeout_time = std::chrono::system_clock::now() + modem_timeout_interval;
        timeout = chrono::system_clock::now() + 5000ms;
        if (modem_buffer_uzed > modem_buffer_max / 2) this_thread::sleep_for(700ms);
    }

    if(timeout > chrono::system_clock::now()) ret = true;



    return ret;
}

bool
azart_cl::close_data_call()
{
    using namespace std;

    bool            ret {false};
    const string    call_clear = "ATH\r\n";
    auto            timeout = chrono::system_clock::now() + 2000ms;

    last_code = AT_WAIT;

    while (current_process_state != IDLE){
        this_thread::sleep_for(std::chrono::milliseconds{1});
        if(timeout < chrono::system_clock::now()) break;
    }

    prepare_tx_frame(data_types::AT_COMM, call_clear.c_str(), call_clear.length() );

    set_process_state(SEND_DATA_FRAME);
    last_code = AT_WAIT;

    while (last_code == AT_WAIT){
        this_thread::sleep_for(chrono::milliseconds{1});
        if(timeout < chrono::system_clock::now()) break;
    }

    if(last_code == AT_OK){
        modem_open = false;
        ret = true;
    }
    return ret;
}

bool
azart_cl::flush_close_data_call()
{
    using namespace std;

    bool            ret {false};
    const string    call_clear = "ATH\r\n";
    const string    get_buff_info = "AT$TXBUF?\r\n";
    auto            timeout = chrono::system_clock::now() + 2000ms;

    do{
        timeout =   chrono::system_clock::now() + 2000ms;
        while (current_process_state != IDLE){
            this_thread::sleep_for(std::chrono::milliseconds{1});
            if(timeout < chrono::system_clock::now()) break;
        }

        last_code = AT_WAIT;

        prepare_tx_frame(data_types::AT_COMM, get_buff_info.c_str(), get_buff_info.length() );

        set_process_state(SEND_DATA_FRAME);

        while (last_code == AT_WAIT){
            this_thread::sleep_for(chrono::milliseconds{1});
            if(timeout < chrono::system_clock::now()) break;
        }

        if(last_code == AT_OK){
            timeout =   chrono::system_clock::now() + 2000ms;
        }


    } while(modem_buffer_uzed);

    // стадия закрытия

    this_thread::sleep_for(600ms); //дать станции закончить передачу

    while (current_process_state != IDLE){
        this_thread::sleep_for(std::chrono::milliseconds{1});
        if(timeout < chrono::system_clock::now()) break;
    }

    prepare_tx_frame(data_types::AT_COMM, call_clear.c_str(), call_clear.length() );

    set_process_state(SEND_DATA_FRAME);
    last_code = AT_WAIT;

    while (last_code == AT_WAIT){
        this_thread::sleep_for(chrono::milliseconds{1});
        if(timeout < chrono::system_clock::now()) break;
    }

    if(last_code == AT_OK){
        modem_open = false;
        ret = true;
    }
    return ret;
}

bool
azart_cl::receive_binary(azart_cl::incoming_data_t &buffer)
{
    using namespace std;
    bool ret {false};

    if(!modem_rx_data.empty() && !modem_open)
    {
        ret = true;
        buffer.second.swap(modem_rx_data);
        buffer.first = modem_caller_id;
    }else if(modem_open){
        while(modem_open){
            this_thread::sleep_for(100ms);
        }
        if(!modem_rx_data.empty() ){
            ret = true;
            buffer.second.swap(modem_rx_data);
            buffer.first = modem_caller_id;
        }
    }
    return ret;
}


#include <map>
void
azart_cl::set_process_state(azart_cl::process_states state_l)
{
    using std::map;
    using std::string;
    static map<int,string> ps_str{{1,"IDLE"},
                                        {2,"WAIT_FRAME"},
                                        {4,"GOT_FRAME"},
                                        {8,"SEND_EMPTY_FRAME"},
                                        {16,"SEND_DATA_FRAME"},
                                        {32,"RETRANSMIT_FRAME"} };
    current_process_state = state_l;
#ifdef GW_DEBUG
    agw_debug2("Set process_state: ", ps_str[state_l]);
#endif
}

void
azart_cl::set_work_state(azart_cl::work_states state_l)
{
    using std::map;
    using std::string;
    static map<int,string> ws_str{{0,"INITIAL_S1"},
                                        {1,"INITIAL_S2"},
                                        {2,"INITIAL_S3"},
                                        {3,"INITIAL_AT_S1"},
                                        {4,"INITIAL_AT_S2"},
                                        {5,"ON_LINK"},
                                        {6,"SEND_SDS_S1"},
                                        {7,"SEND_SDS_S2"},
                                        {8,"ERROR"} };

    current_work_state = state_l;
#ifdef GW_DEBUG
    agw_debug2("Set work_state: ", ws_str[state_l]);
#endif
}
