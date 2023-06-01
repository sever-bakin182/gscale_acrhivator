#include "archiver_gscale.h"

#include <array>
#include <cstring>
#include <cassert>
#include <memory>
#include <algorithm>

namespace gscale {

namespace {
    constexpr uint8_t lim_pix = 4;
    constexpr uint8_t len_identificator = 2;
    constexpr uint8_t len_header = 12; // 2 bytes = identificator, 4 and 4 it's width and height, 2 bytes len or header

    typedef unsigned char pixel_t;
    typedef pixel_t* p_pixel_t;

inline bool is_max32(const uint32_t &v) {
    return (v == 0xffffffff);
};

inline bool is_max(const uint8_t* v, const uint8_t size) {
    return std::all_of(v, v + size, [](const uint8_t byte) {return (byte == 0xff);});
};

inline bool is_min32(const uint32_t &v) {
    return (v == 0);
};

inline bool is_min(const uint8_t* v, const uint8_t size) {
    return std::all_of(v, v + size, [](const uint8_t byte) {return (byte == 0x00);});
};

    ////////////////////////////////////////////////////////////////// start bit_setter
    constexpr std::array<uint8_t, 8> table_1 = {0b00000001, 0b00000010,
                                                0b00000100, 0b00001000,
                                                0b00010000, 0b00100000,
                                                0b01000000, 0b10000000};
    constexpr std::array<uint8_t, 8> table_0 = {0b11111110, 0b11111101,
                                                0b11111011, 0b11110111,
                                                0b11101111, 0b11011111,
                                                0b10111111, 0b01111111};

    class bit_setter {

        uint8_t *m_data;
        size_t m_cur_pos_byte;
        uint8_t m_cur_pos_bit;

    public:
        explicit bit_setter(uint8_t *d)
                : m_data(d)
                , m_cur_pos_byte(0)
                , m_cur_pos_bit(7) {
            assert(m_data != nullptr);

        }

        void set_bit(const uint8_t v) {
            if (v) {
                m_data[m_cur_pos_byte] |= table_1[m_cur_pos_bit];
            } else {
                m_data[m_cur_pos_byte] &= table_0[m_cur_pos_bit];
            }

            shift_to_rignt_bit();
        }

        void set_8(const uint8_t val) {
            if (m_cur_pos_bit == 7) {
                m_data[m_cur_pos_byte] |= val;
                ++m_cur_pos_byte;
            } else {
                uint8_t shift = 7 - m_cur_pos_bit;

                m_data[m_cur_pos_byte] |= (val >> shift);
                ++m_cur_pos_byte;
                m_data[m_cur_pos_byte] |= (val << (8 - shift));
            }
        }

        inline void set_rest(const uint8_t* arr, const uint8_t size) {
            for(uint8_t i = 0; i < size; ++i) {
                set_8(arr[i]);
            }
        }

        template<class T>
        void set_T(const T& valT) {
            const uint8_t* val8 = reinterpret_cast<const uint8_t*>(&valT);
            const uint8_t count = sizeof(T);
            set_rest(val8, count);
        }

        /*void set_32(const uint32_t& val32) {
            const uint8_t* val8 = reinterpret_cast<const uint8_t*>(&val32);
            for(uint8_t i = 0; i < 4; ++i) {
                set_8(val8[i]);
            }
        }*/

        size_t size_bits() const {
            return ((m_cur_pos_byte + 1) * 8 + m_cur_pos_bit + 1);
        }

        size_t size_bytes() const {
            return (m_cur_pos_byte + 1 + ((m_cur_pos_bit == 7)?0:1));
        }

        size_t size_header() const {
            return len_header;
        }

    private:
        inline void shift_to_rignt_bit() {
            if (0 == m_cur_pos_bit) {
                m_cur_pos_bit = 7;
                ++m_cur_pos_byte;
            } else {
                --m_cur_pos_bit;
            }
        }
    };
    ////////////////////////////////////////////////////////////////// end bit_setter
    ////////////////////////////////////////////////////////////////// start bit getter
    class bit_getter {
        const uint8_t *m_data;
        const size_t m_size;
        size_t m_cur_pos_byte;
        uint8_t m_cur_pos_bit;
    public:
        explicit bit_getter(const uint8_t* data, const size_t size)
            : m_data(data)
            , m_size(size)
            , m_cur_pos_byte(0)
            , m_cur_pos_bit(7) {}

        void get_bits_raw(uint8_t* out, const uint32_t len, const uint32_t count_4pixs, const uint8_t rest) {
            if (is_1()) {
                for(uint32_t i = 0; i < count_4pixs; ++i) {
                    get_4pixels(out + i*4);
                }

                if (rest > 0) {
                    get_rest_pixels(out + count_4pixs * 4, rest);
                }
            } else {
                std::memset(out, 0xff, len); //filling white line
            }
        }

    private:
        inline bool is_1() {
            const bool result = m_data[m_cur_pos_byte] & table_1[m_cur_pos_bit];
            shift_to_rignt_bit();
            return result;
        }

        inline void shift_to_rignt_bit() {
            if (0 == m_cur_pos_bit) {
                m_cur_pos_bit = 7;
                ++m_cur_pos_byte;
                assert(m_cur_pos_byte < m_size);
            } else {
                --m_cur_pos_bit;
            }
        };

        inline void get_pixel(uint8_t* out) {
            uint8_t v = m_data[m_cur_pos_byte];
            uint8_t sh = 7 - m_cur_pos_bit;
            v = v << (sh);
            *out |= v;
            v = m_data[++m_cur_pos_byte];
            v = v >> (8 - sh);
            *out |= v;
        };

        inline void get_4pixels(uint8_t* out) {
            get_rest_pixels(out, 4);
        }

        inline void get_rest_pixels(uint8_t* out, const uint8_t size) {
            if (is_1()) { // 1
                if (is_1()) {
                    for(uint8_t i = 0; i < size; ++i) {
                        get_pixel(out + i);
                    }
                } else { // 0
                    std::memset(out, 0x00, size); //filling 4 bytes
                }
            } else {                 // 0
                std::memset(out, 0xff, size); //filling 4 bytes
            }
        }
    };

    ////////////////////////////////////////////////////////////////// end bit getter

    bool is_valid_rid(const RawImageData& rid) {
        return (((rid.width > 0)
                 && (rid.height > 0)
                 && (rid.data != nullptr))? true : false);
    };

    bool is_white_row(const unsigned char* raw, const int count4, const int rest) {
        const uint32_t* raw_32 = reinterpret_cast<const uint32_t*>(raw);
        for (int i = 0; i < count4; ++i) {
            if (!is_max32(raw_32[i]))
                return false;
        }

        if (rest > 0) {
            if (!is_max(raw + count4*4,rest))
                return false;
        }
        return true;
    };

    void fill_row(const unsigned char* raw, const size_t count4, const uint8_t rest, bit_setter& bw) {

        const uint32_t* raw_32 = reinterpret_cast<const uint32_t*>(raw);
        for (size_t i = 0; i < count4; ++i) {

            if (is_max32(raw_32[i])) { // check is white
                bw.set_bit(0);
            } else if (is_min32(raw_32[i])) { //check is black
                bw.set_bit(1);
                bw.set_bit(0);
            } else {                      //is mix pixels
                bw.set_bit(1);
                bw.set_bit(1);
                bw.set_T(raw_32[i]);
            }
        }

        //check rest pixels. For it uses same rule like for 4 pixels
        if (rest > 0) {
            const uint8_t* rest_raw = reinterpret_cast<const uint8_t*>(raw + count4*4);
            if (is_max(rest_raw, rest)) { // check is white
                bw.set_bit(0);
            } else if (is_min(rest_raw, rest)) { //check is black
                bw.set_bit(1);
                bw.set_bit(0);
            } else {                      //is mix pixels
                bw.set_bit(1);
                bw.set_bit(1);
                bw.set_rest(rest_raw, rest);
            }
        }
    };
};

std::size_t compress(RawImageData& data_pic, const std::size_t len_data, InOutData& out) {

    bool result = is_valid_rid(data_pic);
    if (!result) return 0;

    const uint32_t w = data_pic.width;
    const uint32_t h = data_pic.height;
    const uint16_t len_original_header = len_data - (w*h);
    const uint32_t count_4pixs = w/lim_pix; //w = 825
    const uint8_t rest_pixels = w%lim_pix;   //w = 825

    //use only this buffer
    std::unique_ptr<uint8_t[]> out_data(new uint8_t[w*h + rest_pixels*h + count_4pixs*h/4 + h/8 ]); //its max size possible output data.

    unsigned char* p_data = data_pic.data + len_original_header;
    bit_setter bw(out_data.get());

    //compress data of picture
    unsigned char* p_start = p_data;
    unsigned char* p_end = p_data + w*h;
    for(; p_start < p_end; p_start += w) {

        if (is_white_row(p_start, count_4pixs, rest_pixels)) {
            bw.set_bit(0); // set index raw (is empty)
        } else {
            bw.set_bit(1); // set index raw (is fill)
            fill_row(p_start, count_4pixs, rest_pixels, bw);
        }
    }

    //write output buffer
    uint8_t* new_out_data = new uint8_t[len_header + len_original_header + bw.size_bytes()]; //its max size possible output data.
    uint8_t* p_tmp = new_out_data;
    std::memcpy(p_tmp, "BA", len_identificator); //set name identificator = 2 bytes
    p_tmp += len_identificator;
    *reinterpret_cast<uint32_t*>(p_tmp) = w;
    p_tmp += sizeof(w);
    *reinterpret_cast<uint32_t*>(p_tmp) = h;
    p_tmp += sizeof(h);
    *reinterpret_cast<uint16_t*>(p_tmp) = len_original_header; // set size of original header data = 2 bytes
    p_tmp += sizeof(len_original_header);
    std::memcpy(p_tmp, data_pic.data, len_original_header); // set data original header
    p_tmp += len_original_header;

    std::memcpy(p_tmp, out_data.get(), bw.size_bytes());

    out.data = new_out_data;
    out.size = len_header + len_original_header + bw.size_bytes();

    return out.size;
};


std::size_t decompress(RawImageData &data_pic, InOutData &in) {
    if (!in.data || !in.size) return 0;

    uint8_t* p_idata = in.data;
    long unsigned int size_idata = in.size;

    p_idata += len_identificator; //name identificator = 2 bytes //skip
    const uint32_t w = *reinterpret_cast<uint32_t*>(p_idata); //get width = 4 bytes
    p_idata += sizeof (w);
    const uint32_t h = *reinterpret_cast<uint32_t*>(p_idata); //get height = 4 bytes
    p_idata += sizeof (h);
    const uint16_t len_original_header = *reinterpret_cast<uint16_t*>(p_idata); //get len of original header
    p_idata += sizeof(len_original_header);

    unsigned char *p_odata = new unsigned char[len_original_header + w*h];
    std::memcpy(p_odata, p_idata, len_original_header);

    p_idata += len_original_header;
    bit_getter bg(p_idata, size_idata - len_header - len_original_header);

    const uint32_t count4 = w/lim_pix;
    const uint8_t rest = w%lim_pix;

    unsigned char *p_start = p_odata + len_original_header;
    const unsigned char *p_end = p_start + w*h;
    for (; p_start < p_end; p_start += w) {
        bg.get_bits_raw(p_start, w, count4, rest);
    }

    data_pic.width = w;
    data_pic.height = h;
    data_pic.data = p_odata;

    return len_original_header + w*h;
};

void freeOutputData(InOutData& od) {
    delete [] od.data;
    od.data = nullptr;
    od.size = 0;
};

};
