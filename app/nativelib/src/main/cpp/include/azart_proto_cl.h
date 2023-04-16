// Copyright (c) 2022 OOO FIRMA 'NOVOPLAN' . All rights reserved.
// Author: Vladimir Guzenko, <vg@mptsrv.com>

#ifndef AZART_PTOTO_AZART_PROTO_CL_H
#define AZART_PTOTO_AZART_PROTO_CL_H
#include <cinttypes>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

#ifdef __cplusplus
extern "C" {
#endif


class azart_cl {
    using system_clock = std::chrono::system_clock;
    using ms_ = std::chrono::milliseconds;
    const ms_ poll_interval{10};
    const ms_ short_poll_interval{2};
    const ms_ link_error_interval{9000};
    const ms_ packet_error_interval{500};
    const ms_ modem_timeout_interval{5000};
public:
    typedef std::pair<unsigned, std::vector<uint8_t>> incoming_data_t;
    enum {
        START_BYTE = 0xC0,
        STOP_BYTE = 0xC0
    };
    enum frame_types {
        INIT_STATE,
        DATA_FRAME,
        EMPTY_FRAME,
        AT_WAIT_R
    };
    enum data_types {
        RADIO_DATA = 3,
        AT_COMM = 5,
        DATA_RP_KCC = 6,
        DATA_KCC_RP = 7
    };
    enum link_states {
        LINK_WAIT,
        DB_GOT,
        START_GOT,
        END_GOT
    };
    enum work_states : int{
        INITIAL_S1, // Состояние установки и подготовки станции к работе скорость 9600 отправка пустого фрейма
        INITIAL_S2, // Состояние установки: отправка ATS23=230400
        INITIAL_S3, // Состояние установки: Изменение собственной скорости 230400
        INITIAL_AT_S1,
        INITIAL_AT_S2,
        ON_LINK, // Нормальное состояние
        SEND_SDS_S1,
        SEND_SDS_S2,
        ERROR    // Сброс и переход к начальному состоянию
    };
    enum process_states : int{
        IDLE = 1,
        WAIT_FRAME = 2,     // отправили и ожидаем ответа
        GOT_FRAME = 4,      // принят новый фрейм, не обработан
        SEND_EMPTY_FRAME = 8,
        SEND_DATA_FRAME = 16,
        RETRANSMIT_FRAME = 32
    };
    enum sds_send_states : int {
        SEND_FAIL = 0,
        SEND_OK = 1,
        SEND_INIT = 2
    };
    enum at_codes :int{
        AT_WAIT,
        AT_ERROR,
        AT_OK,
        AT_CONNECT
    };
    enum data_rates : unsigned {
        RATE_7200_UNPROTECTED = 1,
        RATE_4800_LOW_PROTECT_INTERLEAVE_1,
        RATE_4800_LOW_PROTECT_INTERLEAVE_4,
        RATE_4800_LOW_PROTECT_INTERLEAVE_8,
        RATE_2400_HI_PROTECT_INTERLEAVE_1,
        RATE_2400_HI_PROTECT_INTERLEAVE_4,
        RATE_2400_HI_PROTECT_INTERLEAVE_8,
    };
    enum modem_frame_sizes : int{
        SRATE_7200 = 54,//
        SRATE_4800 = 36,//
        SRATE_2400 = 18//
    };
private:
    struct __attribute((packed, aligned(1))) frame_header_s {
        uint8_t index;
        uint8_t address;
        uint8_t data_len_h;
        uint8_t data_len_l;
    };
    struct __attribute((packed, aligned(1))) frame_data_header_s {
        uint8_t type;
        uint8_t data_len_h;
        uint8_t data_len_l;
    };
    struct __attribute((packed, aligned(1))) rp_kcc_data_header_s {
        uint8_t type;
        uint8_t data_len_h;
        uint8_t data_len_l;
        uint8_t bytes_used;
    };
    struct frame_s {
        frame_header_s *header;
        frame_data_header_s *data_header;
        uint8_t *frame_data;
    };

    static uint8_t empty_frame[6];
    static const uint16_t crc_table[256];


    link_states                 current_link_state;
    std::atomic<work_states>    current_work_state;
    std::atomic<process_states> current_process_state;
    std::atomic<bool>           modem_open {false};
    std::atomic<data_rates>     modem_rate {RATE_7200_UNPROTECTED};
    std::atomic<modem_frame_sizes>
                                modem_frame_size {SRATE_7200};
    std::atomic<unsigned>       modem_buffer_uzed   {0};
    std::atomic<unsigned>       modem_buffer_max    {0};
    std::atomic<unsigned>       modem_caller_id     {0};
    std::atomic<at_codes>       last_code {AT_ERROR};
    frame_types                 current_rx_frame_type;
    unsigned int                buff_pos;
    unsigned int                frame_index;
    std::chrono::system_clock::time_point
                                radio_poll_time;
    std::chrono::system_clock::time_point
                                link_error_time;
    std::chrono::system_clock::time_point
                                packet_error_time;
    std::chrono::system_clock::time_point
                                modem_timeout_time;
    std::vector<uint8_t>        modem_rx_data;
    unsigned                    last_frame_index{0};
    frame_types                 last_frame_type{EMPTY_FRAME};
    uint8_t                     raw_recived_frame[4096];
    uint8_t                     tx_frame[4096];
    uint8_t                     sds_from_num[64];
    uint8_t                     sds_string[1024];
    uint8_t                     sds_iso_string[1024];
    uint8_t                     tx_frame_len;
    std::atomic<uint8_t>        sds_ready;
    std::atomic<int>            sds_send_status {SEND_FAIL};
    uint16_t                    sds_message_len;
    uint16_t                    self_number;

    int (*rx_f)(char *);
    int (*tx_fm)(std::vector<char>);

    uint16_t crc_calc(const void *buffer, unsigned int count, uint16_t init_crc = 0xFFFFU) {
        unsigned int i;
        auto p = (unsigned char *) buffer;

        for (i = 0; i < count; i++) {
            init_crc = crc_table[((init_crc >> 8) & 0xFF) ^ p[i]] ^ (init_crc << 8);
        }

        return (init_crc);
    }

    int crc_check();

    void process_frame();

    void process_byte(uint8_t b);

    void main_loop();

    void initial_setings();// устанавливает соединение со станцией

    inline
    void radio_port_change_baud(uint32_t baud);

    void send_empty_frame();

    void send_data_frame();

    void write_to_dataport(uint8_t *d, size_t len);

    void prepare_tx_frame(data_types frame_data_type, const void *frame_data, size_t data_len);

    void msm_decode(char *dst, char *msg);


    void msm_encode(char *dst, char *msg);

    void send_raw_at();

    void utils_set_modem_param_on_invite(char *at_comm, size_t len);

    void set_process_state(process_states state);
    void set_work_state(work_states state);


public:


    azart_cl() :
        current_link_state(link_states::LINK_WAIT),
        current_work_state(work_states::INITIAL_AT_S1),
        current_process_state(process_states::IDLE),
        current_rx_frame_type(frame_types::INIT_STATE),
        buff_pos(0),
        frame_index(0),
        sds_ready(0),
        sds_message_len(0),
        self_number(0)
        {
        tx_frame[0] = frame_index;// index
        tx_frame[1] = 0x01;// РС485 номер станции  TODO
        radio_poll_time = system_clock::now() + poll_interval ;
        link_error_time = system_clock::now() + link_error_interval;
        packet_error_time = system_clock::now() + packet_error_interval;

    }
    ~azart_cl(){}

    /*!
        * \brief назначает функцию для приема данных
        * char* указатель на переменную типа чар
        * \param указатель на фукнцию int f(char*))
    */
    void set_rx_func(int(*)(char*));

    /*!
        * \brief назначает функцию для передачи данных
        * \param указатель на фукнцию int f(std::vector<char>))
    */
    void set_txm_func(int(*)(std::vector<char>));

    /*!
        * \brief Двтгатель прогресса ;)
        * дёргать каждые 10 мс!.
    */
    void poll();

    /*!
        * \brief Отправляет короткое текстовое сообщение
        * \param string указатель на фукнцию нуль терминированную строку
        * \retval статус 1 - ОК / 0 - НОК
    */
    int send_sds( char const *string);

    /*!
        * \brief Открывает канал связи с указанными параметрами
        * \param rate Скорость передачи см azart_cl::data_rates
        * \param dest_radio_num номер абонентской радиостанции
        * \param simplex тип соединения ( не реализованно, использовать параметр по умолчанию)
        * \retval статус  ОК / НОК
    */
    bool init_data_call(data_rates rate, int dest_radio_num,  bool simplex = true);

    /*!
        * \brief Пытается прервать модемный вызов немедленно
        * \retval статус 1 - ОК / 0 - НОК
    */
    bool close_data_call();


    /*!
        * \brief Дожидается отправки всех данных с РС и прерывает вызов
        * \retval статус 1 - ОК / 0 - НОК
    */
    bool flush_close_data_call();


    /*!
        * \brief Отправляет данные через открытый канал
        * данные разбиваются на пакеты в зависимости от настройки скорости соединения
        * см azart_proto::modem_frame_sizes
        * \param data данные для отправки
        * \retval статус 1 - ОК / 0 - НОК
    */
    bool send_binary(std::vector<uint8_t> data);

    /*!
        * \brief Блокирует вызов до закрытия модема
        *        заполняет буфер входящими модемными данными
        * \param buffer (std::pair<unsigned, std::vector<char>>)
        *        ссылка на буфер подлежащий заполнению
        *        first - caller id
        *        second - incoming payload
        * \retval статус 1 - Если данные были / 0 - Если внутренний буфер пуст
    */
    bool receive_binary(azart_cl::incoming_data_t &buffer);


    /*!
    * \brief Возвращает статус модема.
    * \retval статус true если открыт канал связи, иначе false
    */
    bool is_modem_open() const { return modem_open; };

    /*!
    * \brief Проверяет наличие новых данных. Не гарантирует, что данные
     * будут отданы пока модем не закроется.
    * \retval статус true если есть новые данные, иначе false
    */
    bool is_data_available() const { return !modem_rx_data.empty(); };

    /*!
        * \brief Возвращает принятое сообщение или пустой вектор
        * \retval std::vector<char> содержащий принятое сообщение
    */
    std::vector<char> get_sds();


    /*!
        * \brief Возвращает статус оправки текстового сообщения
        * \retval статус 1 - Отправлено в эфир / 0 - Сбой отправки
    */
    int get_send_status();

    /*!
        * \brief Возвращает номер своей радиостанции
        * \retval Номер своей радиостанции
    */
    uint16_t get_self_number() const
    {
        return self_number;
    }
};

#ifdef __cplusplus
}
#endif

#endif //AZART_PTOTO_AZART_PROTO_CL_H
