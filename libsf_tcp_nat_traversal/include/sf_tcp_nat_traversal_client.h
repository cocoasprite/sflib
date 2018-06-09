#pragma once

#include "sf_nocopy.h"
#include "sf_object.h"
#include "sf_tcpclient.h"
#include "sf_tcpserver.h"
#include "sf_serialize.h"
#include "sf_define.h"
#include "sf_tcp_nat_traversal_utils.h"
#include "sf_random.h"
#include <climits>


namespace skyfire {

    //p2p客户端连接上下文
    struct sf_p2p_connect_context_t__{
        sf_tcp_nat_traversal_context_t__ tcp_nat_traversal_context;
        std::shared_ptr<sf_tcpclient> point_a_client;
        std::shared_ptr<sf_tcpserver> point_a_server;
        std::shared_ptr<sf_tcpclient> point_b_client;
        std::shared_ptr<sf_tcpserver> point_b_server;
    };

    class sf_tcp_nat_traversal_client : public sf_nocopy<sf_object> {
    private:
        std::shared_ptr<sf_tcpclient> client__ {sf_tcpclient::make_client()};
        std::set<unsigned long long> client_list__;
        unsigned long long self_id__;
        std::map<int, sf_p2p_connect_context_t__> connnect_context_map__;
        addr_info_t server_addr__;


        void on_client_data_coming__(const pkg_header_t &header, const byte_array &data){
            switch (header.type){
                case TYPE_NAT_TRAVERSAL_LIST:
                    sf_deserialize(data, client_list__, 0);
                    break;
                case TYPE_NAT_TRAVERSAL_SET_ID:
                    sf_deserialize(data, self_id__, 0);
                    break;
                default:
                    break;
            }
        }

        void on_client_close__(){
            client_list__.clear();
        }



    public:

        static std::shared_ptr<sf_tcp_nat_traversal_client> make_client(){
            return std::make_shared<sf_tcp_nat_traversal_client>();
        }


        sf_tcp_nat_traversal_client(){
            sf_bind_signal(client__, data_coming, std::bind(&sf_tcp_nat_traversal_client::on_client_data_coming__,
                                                            this,
                                                            std::placeholders::_1,
                                                            std::placeholders::_2),false);
            sf_bind_signal(client__, closed, std::bind(&sf_tcp_nat_traversal_client::on_client_close__, this), false);
        }

        /**
         * 连接到穿透服务器，连接成功后会注册
         * @param ip 穿透服务器ip
         * @param port 穿透服务器端口
         * @return 连接结果
         */
        bool connect(const std::string& ip, unsigned short port){
            server_addr__.ip = ip;
            server_addr__.port = port;
            if(client__->connect(ip, port))
            {
                client__->send(TYPE_NAT_TRAVERSAL_REG, byte_array());
                return true;
            }
            return false;
        }

        /**
         * 获取终端集合
         * @return 终端集合
         */
        std::set<unsigned long long> get_clients(){
            return client_list__;
        }

        /**
         * 连接远程peer端
         * @param peer_id 远端id
         * @return -1表示失败，其他表示此次连接的id
         */
        int connect_to_peer(unsigned long long peer_id){
            // 生成一个连接上下文
            sf_p2p_connect_context_t__ tmp_p2p_conn_context;
            // 生成随机连接id
            tmp_p2p_conn_context.tcp_nat_traversal_context.connect_id = sf_random::get_instance()->get_int(0, INT_MAX);
            // 指定对方id
            tmp_p2p_conn_context.tcp_nat_traversal_context.dest_id = peer_id;
            // 指定自身id
            tmp_p2p_conn_context.tcp_nat_traversal_context.src_id = self_id__;
            // 生成连接a客户端
            tmp_p2p_conn_context.point_a_client = sf_tcpclient::make_client();
            // 尝试连接服务器
            if(tmp_p2p_conn_context.point_a_client->connect(server_addr__.ip,server_addr__.port)) {
                // 发起连接请求
                if (tmp_p2p_conn_context.point_a_client->send(TYPE_NAT_TRAVERSAL_REQUIRE_CONNECT_PEER,
                                   sf_serialize(tmp_p2p_conn_context.tcp_nat_traversal_context))) {
                    // 保存连接上下文
                    connnect_context_map__[tmp_p2p_conn_context.tcp_nat_traversal_context.connect_id] = tmp_p2p_conn_context;
                    return tmp_p2p_conn_context.tcp_nat_traversal_context.connect_id;
                }
            }
            return  -1;
        }

        void close(){
            client__->close();
        }
    };
}