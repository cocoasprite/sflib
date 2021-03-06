
/**
* @version 1.0.0
* @author skyfire
* @mail skyfireitdiy@hotmail.com
* @see http://github.com/skyfireitdiy/sflib
* @file sf_tcp_client_linux.hpp

* sflib第一版本发布
* 版本号1.0.0
* 发布日期：2018-10-22
*/

#pragma once

#include "sf_tcp_client_linux.h"
#include "sf_tcp_utils.hpp"
#include "sf_object.hpp"
#include "sf_nocopy.h"
#include "sf_type.hpp"
#include "sf_tcp_client_interface.hpp"

namespace skyfire
{
    inline sf_tcp_client::sf_tcp_client(bool raw) {
        sock__ = socket(AF_INET, SOCK_STREAM, 0);
        if (sock__ == -1)
        {
            inited__ = false;
            return;
        }
        int opt = 1;
        if (-1 == setsockopt( sock__, SOL_SOCKET,SO_REUSEADDR,
                              reinterpret_cast<const void *>(&opt), sizeof(opt))){
            inited__ = false;
            return ;
        }
        inited__ = true;
        raw__ = raw;
    }

    inline SOCKET sf_tcp_client::get_raw_socket() {
        return sock__;
    }

    inline bool sf_tcp_client::bind(const std::string &ip, unsigned short port) {
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(ip.c_str());
        address.sin_port = htons(port);
        return -1 != ::bind(sock__,reinterpret_cast<sockaddr*>(&address), sizeof(address));
    }

    inline std::shared_ptr<sf_tcp_client> sf_tcp_client::make_client(bool raw) {
        return std::make_shared<sf_tcp_client>(raw);
    }

    inline sf_tcp_client::~sf_tcp_client() {
        close();
    }

    inline bool sf_tcp_client::connect_to_server(const std::string &ip, unsigned short port) {
        if (!inited__)
            return false;
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(ip.c_str());
        address.sin_port = htons(port);
        if (::connect(sock__, reinterpret_cast<const sockaddr *>(&address), sizeof(address)) != 0)
        {
            return false;
        }
        std::thread([=]
                    {
                        byte_array recv_buffer(SF_NET_BUFFER_SIZE);
                        byte_array data;
                        sf_pkg_header_t header;
                        while (true)
                        {
                            auto len = read(sock__, recv_buffer.data(), SF_NET_BUFFER_SIZE);
                            if (len <= 0)
                            {

                                closed();
                                break;
                            }
                            if(raw__)
                            {
                                raw_data_coming(byte_array(recv_buffer.begin(),recv_buffer.begin() + len));
                            }
                            else
                            {
                                data.insert(data.end(), recv_buffer.begin(), recv_buffer.begin() + len);
                                size_t read_pos = 0;
                                while (data.size() - read_pos >= sizeof(sf_pkg_header_t))
                                {
                                    std::memmove(&header, data.data() + read_pos, sizeof(header));
                                    if (!check_header_checksum(header))
                                    {
                                        close();
                                        return;
                                    }
                                    if (data.size() - read_pos - sizeof(header) >= header.length)
                                    {
                                        data_coming(
                                                header,
                                                byte_array(
                                                        data.begin() + static_cast<long>(read_pos) + sizeof(header),
                                                        data.begin() + static_cast<long>(read_pos) + sizeof(header)
                                                        + static_cast<long>(header.length)));
                                        read_pos += sizeof(header) + header.length;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                                if (read_pos != 0)
                                {
                                    data.erase(data.begin(), data.begin() + static_cast<long>(read_pos));
                                }
                            }
                        }
                    }).detach();
        return true;
    }

    inline bool sf_tcp_client::send(int type, const byte_array &data) {
        if (!inited__)
            return false;
        sf_pkg_header_t header;
        header.type = type;
        header.length = data.size();
        make_header_checksum(header);
        auto ret = write(sock__, make_pkg(header).data(), sizeof(header));
        if (ret != sizeof(header))
            return false;
        return write(sock__, data.data(), data.size()) == static_cast<ssize_t>(data.size());
    }

    inline bool sf_tcp_client::send(const byte_array &data) {
        if (!inited__)
            return false;
        return write(sock__, data.data(), data.size()) == static_cast<ssize_t>(data.size());
    }

    inline void sf_tcp_client::close() {
        if (!inited__)
            return;
        shutdown(sock__,SHUT_RDWR);
        ::close(sock__);
        sock__ = -1;
    }
}
