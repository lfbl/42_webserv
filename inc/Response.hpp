/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfabel <lfabel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/23 10:50:06 by lfabel            #+#    #+#             */
/*   Updated: 2025/08/27 16:03:54 by lfabel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../inc/CGIHandler.hpp"
#include "../inc/Routing.hpp"
#include <ctime>
#include <vector>
#include "init.hpp"


class CGIHandler;

std::string trim_response(std::string buf, size_t pos);
size_t find_body(std::string buf, size_t pos);
std::string createHeader(const std::string& body, Routing::RequestInfo& reqInfo);
std::string makeResponse(std::string &full_response, Routing::RequestInfo& reqInfo);
std::string findAttributes(std::string response);
std::string sendErrorResponse(int status, Routing::RequestInfo& reqInfo);
std::string executeStatic(std::string path, Routing::RequestInfo& reqInfo);
std::vector<std::string> getData(std::string path);
void findVariables(const std::string& str, Routing::RequestInfo& reqInfo);
std::string vectoStr(std::vector<std::string>& files,std::string& path);

#endif