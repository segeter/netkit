#include <netkit/http/context.h>
#include <netkit/http/router.h>

#include <cstdlib>
#include <iostream>
#include <thread>

using namespace netkit;

using HttpContextPtr = std::shared_ptr<netkit::http::Context>;
using Router = netkit::http::Router;

static std::string funcname;

static void OnHello(const HttpContextPtr& ctx) {
  funcname = __FUNCTION__;
  std::cout << __FUNCTION__ << std::endl;
}

static void OnHelloPath(const HttpContextPtr& ctx, const std::string& name) {
  funcname = __FUNCTION__;
  std::cout << __FUNCTION__ << " name=" << name << std::endl;
}

static void OnHelloArg(const HttpContextPtr& ctx, const std::string& name,
                       const std::optional<std::string>& nick_name,
                       std::int32_t age) {
  funcname = __FUNCTION__;
  std::cout << __FUNCTION__ << " name=" << name
            << " nick_name=" << (nick_name ? nick_name->c_str() : "")
            << " age=" << age << std::endl;
}

void TestHttpRouter(std::stop_token st) {
  Router router;

  try {
    // 添加路由的时候参数顺序必须和回调参数顺序一致!
    router.AddRoute("/hello?name&nick_name&age", &OnHelloArg, {"GET"});
    router.AddRoute("/hello/{name}", &OnHelloPath, {"GET"});
    router.AddRoute("/hello", &OnHello, {"GET", "POST"});
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    throw;
  }

  struct UrlMapping {
    const char* method;
    const char* url;
    const char* func;
  };

  UrlMapping kUrls[] = {
      {"GET", "/hello", "OnHello"},
      {"GET", "/hello?name=xxx&age=34", "OnHelloArg"},
      {"GET", "/hello?nick_name=xxx&name=yyy&age=18", "OnHelloArg"},
      {"GET", "/hello/xxx", "OnHelloPath"},
      {"GET", "/hello?name=yyy&age=18&other", "OnHelloArg"},
      {"GET", "/hello?name1=xxx", "OnHello"},
      {"GET", "/hello?nick_name=xxx", "OnHello"},
      {"GET", "/hello1", ""},
      {"GET", "/hello/xxx/yyyy", ""},
      {"GET", "/hello?name=yyy&age=bad", ""},
      {"POST", "/hello", "OnHello"},
      {"POST", "/hello?name=xxx&age=34", "OnHello"},
      {"POST", "/hello?nick_name=xxx&name=yyy&age=18", "OnHello"},
      {"POST", "/hello/xxx", ""},
      {"POST", "/hello?name=yyy&age=18&other", "OnHello"},
      {"POST", "/hello?name1=xxx", "OnHello"},
      {"POST", "/hello?nick_name=xxx", "OnHello"},
      {"POST", "/hello1", ""},
      {"POST", "/hello/xxx/yyyy", ""},
      {"POST", "/hello?name=yyy&age=bad", "OnHello"},
  };

  HttpContextPtr ctx;
  std::srand((unsigned int)std::time(nullptr));
  while (!st.stop_requested()) {
    funcname = "";
    auto idx = std::rand() % (sizeof(kUrls) / sizeof(UrlMapping));
    auto& item = kUrls[idx];
    std::cout << "-----------------" << std::endl;
    std::cout << item.method << " " << item.url << std::endl;
    try {
      router.Routing(ctx, item.method, item.url);
    } catch (const std::exception& e) {
      std::cout << e.what() << std::endl;
    }
    if (item.func != funcname) {
      throw std::runtime_error(funcname);
    }
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
  }
}
