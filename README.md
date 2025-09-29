# SimjsDelivr 用C++编写的简单的jsdelivr实现
## 实现
### 已实现的功能
* 按文档要求解析链接并返回入口文件，文件列表、特定的文件或404错误  
* 未指明版本号的，自动解析到最新版本  
* 基于修改时间的缓存系统  
### 实现思路
首先确定 registry url ，并创建 cache 文件初始化缓存系统，并新建线程周期性调用 check_cache() 检查缓存情况。使用 [cpp-httplib](https://github.com/yhirose/cpp-httplib) 来处理请求， URL 路径先被解析出包名，然后用 [libcurl](https://curl.se/libcurl/) 从指定的 registry 获取元数据，并将原始的 URL 路径和响应一起传递给 response_parse() ，该函数用正则表达式提取出版本号和文件路径并判断版本/文件是否存在以及是否需要返回文件列表，并用 [nlohmann/json](https://github.com/nlohmann/json) 解析对应版本的 tarball 下载地址并返回。server() 酱会检查指定的包在缓存中是否存在，若不存在就下载，存在就更新 tarball 的修改时间来延长缓存的过期时间。然后临时地解压 tarball。server() 酱继续根据 response_parse() 的返回值返回对应的内容，向用户返回对应的内容之后删除临时解压的文件。  
#### 简单的缓存系统
检查 tarball 的修改时间，超过一段时间无访问的会自动删除以节省空间。缓存的过期时间和检查缓存的间隔时间都定义在 main.h 里。
### 一些问题
* 在解析 URL 时选择了正则表达式，实现简单但可读性并不高，编写正则表达式难度高，依赖AI。且扩展性不高，对特殊格式解析支持是地狱难度的（）（还是看看隔壁的 [boost/url](https://github.com/boostorg/url) 吧（   
* 缓存并未覆盖 npm 包的元数据，把元数据保存为文件并加入缓存管理即可。  
* 即使对于已在缓存中的文件，每次访问都需要重新全部解压，而只保留 tarball 可以减少空间占用。可以考虑只解压特定文件/通过 libarchive 获取文件列表来减少解压时间  
* 缓存实现简单，但是依赖于文件系统保存最后的修改时间，未来会考虑将缓存数据库写入json文件来避免对文件系统的依赖  
* 项目语言选择了 C++ ，但写起来感觉十分复杂（人太菜）。下次不敢了😭  
## 生成式人工智能声明
使用生成式人工智能生成的代码均已标出
### cache_system.cpp
to_unix_timestamp() 和 from_unix_timestamp() 使用 GPT-5 生成，在 file_time_type 和int64_t之间转换，便于计算
### decompressor.cpp
解压部分调用 libarchive ,索性直接全部使用 GPT-5 生成
### file_net_stream.cpp
send_and_delete_file()使用 GPT-5 生成，实现文件传输以达到用户收到文件后立即删除解压后的文件的目的。单纯的使用 res.set_content() 会导致用户收到文件之前已解压的文件就已被删除。
### get_content_type.cpp
无序图由 GPT-5 生成，提供 content-type 的索引
### response_parser.cpp 和 main.cpp
仅正则表达式由 GPT-5 生成
## 心路历程
这是我第一次接触后端，第一次拿C++写超过100行多文件项目（还是太菜了）  
前期 response_parse() 写的我怀疑人生，但是看到第一个成功解析出来的入口文件显示在浏览器还是很有成就感的。  
感谢 Open AI GPT-5 、 Google Gemini 2.5 Pro 、Stack overflow 和 cppreference 帮助我查找资料，感谢 Rikka 在我摸鱼的时候和我闲聊（）让我没那么寂寞。