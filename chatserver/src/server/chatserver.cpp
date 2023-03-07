#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"

#include <iostream>
#include <functional>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

// 初始化聊天服务器对象，构造TCPServer
ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg),
      _loop(loop)
{
    // 注册链接回调
    _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));

    // 注册消息回调
    _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置服务器端的线程数量 1个I/O线程（监听新用户的连接事件）， 3个worker线程
    // 不设置的话，就1个线程而已，要处理连接又要处理业务
    _server.setThreadNum(4);
}

// 启动服务
void ChatServer::start()
{
    _server.start(); // 启动TCPServer
}

// 上报链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    // 客户端断开链接
    if (!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn); // 客户端异常关闭
        conn->shutdown();
    }
}

// 上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
    string buf = buffer->retrieveAllAsString(); // 收到的数据放到这个字符串中

    // cout << "recv data:" << buf << " time:" << time.toFormattedString() << endl;
    // conn->send(buf);//返回 ，收到什么发送什么 

    json js = json::parse(buf); // 解析json字符串
    // 达到的目的：完全解耦网络模块的代码和业务模块的代码
    // 通过js["msgid"] 获取=》业务handler处理器（在业务模块事先绑定好的）=》conn  js  time传给你
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    // 回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn, js, time);
}