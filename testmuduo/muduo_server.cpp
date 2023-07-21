#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<iostream>
#include<functional>
#include<string>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;//bind

//定义自己的服务器类
class ChatServer
{
public:
    /*
    TcpServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg,
            Option option = kNoReusePort);
    */
   ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg)
            //调用TcpServer的有参构造，其无默认构造
            : _server(loop, listenAddr, nameArg), _loop(loop){
                //给服务器注册用户连接的创建和断开回调
                /*
                void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }
需要的类型ConnectionCallback
 typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
 typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
 所以自定义这样的回调类型函数，去处理对应的业务
                */
               //这个自定义的回调函数，需要绑定器，要不然会多一个参数this
                //_server.setConnectionCallback(bind(onConnection, this, _1));
                //也可以这个函数前面加类名访问，并且这样的函数名，需要前面加&
                _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));

                //给服务器注册用户读写事件回调
                /*
                 void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }
                
                typedef std::function<void (const TcpConnectionPtr&,
                            Buffer*,
                            Timestamp)> MessageCallback;
                */
                //_server.setMessageCallback(bind(onMessage, this, _1, _2, _3));
                _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));            
                
                //设置服务端的线程数量
                _server.setThreadNum(4);
            }
    
    //开启事件循环
    void start(){
        _server.start();
    }
    

private:
    
    //专门处理用户的连接创建和断开的业务 epoll listenfd accept
    void onConnection(const TcpConnectionPtr& conn){
        // TcpConnectionPtr是一个智能指针类
        cout << conn->peerAddress().toIpPort() << " -> "<< \
        conn->localAddress().toIpPort() << endl;
        //连接是否断开
        if(conn->connected()){
            cout << conn->peerAddress().toIpPort() << " -> "<< conn->localAddress().toIpPort() << \
            " state : online " << endl;
        }
        else{
            cout << conn->peerAddress().toIpPort() << " -> "<< conn->localAddress().toIpPort()<< \
            "state : offline " << endl;
            //添加上服务器关闭以及事件循环结束
            conn->shutdown();
            //_loop->quit();
        }
    }

    //专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buffer,
                   Timestamp time){
        string buf = buffer->retrieveAllAsString();
        cout << " recv data: " << buf << " time " << time.toString() << endl;
        conn->send(buf);            
    }
    //很多类都是在muduo的命名空间里的
    muduo::net::TcpServer _server;
    //常常用指针形式
    muduo::net::EventLoop* _loop; 

};


//主函数
int main(){
    //定义出需要传的参数
    EventLoop loop;
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();//listenfd epoll_ctl => epoll
    loop.loop(); // epoll_wait()以阻塞方式等待新用户连接，已连接用户的读写事件等

    return 0;
}