
/**
* @version 1.0.0
* @author skyfire
* @mail skyfireitdiy@hotmail.com
* @see http://github.com/skyfireitdiy/sflib
* @file sf_http_request.h

* sflib第一版本发布
* 版本号1.0.0
* 发布日期：2018-10-22
*/


#pragma once

#include <sstream>
#include <utility>
#include <string>
#include "sf_type.h"
#include "sf_http_request_line.h"
#include "sf_utils.h"
#include "sf_http_header.h"
#include "sf_http_utils.h"
#include "sf_logger.h"

namespace skyfire{
    /**
     * @brief  http请求
     */
    class sf_http_request
    {
    private:
        const byte_array raw__;
        bool valid__ = false;
        sf_http_request_line request_line__;
        sf_http_header header__;
        byte_array body__;
        bool multipart_data__ = false;
        sf_multipart_data_context_t multipart_data_context__;
        bool parse_request__();
        std::map<std::string,std::string> cookies__;

    public:

        /**
         * 构造函数
         * @param raw 原始数据
         */
        explicit sf_http_request(byte_array raw);

        /**
         * 构造函数
         * @param multipart_data 分块数据集合
         */
        explicit sf_http_request(sf_multipart_data_context_t multipart_data);

        /**
         * 判断请求是否合法
         * @return 是否合法
         */
        bool is_valid() const;

        /**
         * 获取请求行
         * @return 请求行对象
         */
        sf_http_request_line get_request_line() const;

        /**
         * 获取请求头
         * @return 请求头对象（不包括cookie）
         */
        sf_http_header get_header() const;

        /**
         * 获取请求体（在非分块请求可用）
         * @return 请求体数据
         */
        byte_array get_body() const;

        /**
         * 判断是否是分块请求
         * @return 是否是分块请求
         */
        bool is_multipart_data() const;

        /**
         * 获取分块请求的分块集合（在分块请求可用）
         * @return 分块集合
         */
        sf_multipart_data_context_t get_multipart_data_context() const;

        /**
         * 获取cookies
         * @return cookies
         */
        std::map<std::string,std::string> get_cookies() const;

        /**
         * 分割请求
         * @param raw 原始数据
         * @param request_line 请求行原始数据
         * @param header_lines 请求头原始数据（一行一个）
         * @param body 请求体数据
         * @return 是否分割成功
         */
        static bool split_request(const byte_array &raw,std::string &request_line, std::vector<std::string> &header_lines, byte_array &body);

        /**
         * 解析请求行数据
         * @param request_line 原始请求行数据（来自split_request）
         * @param request_line_para 请求行函数
         * @return 是否解析成功
         */
        static bool parse_request_line(const std::string& request_line,sf_http_request_line& request_line_para);

        /**
         * 解析请求头
         * @param header_lines 请求头原始数据，一行一个（来自split_request）
         * @param header 请求头
         * @return 是否解析成功
         */
        static bool parse_header(const std::vector<std::string> header_lines, sf_http_header& header);

        /**
         * 解析cookies
         * @param header_data 头数据
         * @param cookies cookie
         */
        static void parse_cookies(const sf_http_header &header_data, std::map<std::string,std::string>& cookies);
    };
}