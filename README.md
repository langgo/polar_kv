# polar_kv

https://zhuanlan.zhihu.com/p/33389807
https://yq.aliyun.com/articles/409102

https://zhuanlan.zhihu.com/p/32102406

https://zhuanlan.zhihu.com/p/31664488

https://zhuanlan.zhihu.com/p/32148445

https://zhuanlan.zhihu.com/p/35925589

## Question

日志按块写，按块读。有什么好处？ TODO need实验
- 真的能够提高性能？
- 对 crc32 有什么好处吗？

在系统调用次数和内存复制的二选一？bufio
- write
- buf write

## TODO

整理整个流程，并思考哪些可以聚合（聚合的意义是，并发的情况下也不一定快）

定量描述系统调用的速度

RocksDB 中描述可以并发insert SkipList ? 这个是怎么实现的

## mark

- 不进行 rehash，hash 阶段不需要锁
- 怎么保持不可变状态，这样就不需要每次重头新建索引，加快启动速度。正常的db，肯定都必须做这个事情。
- 是否考虑多个文件，如果需要整理 wal 的时候，确实多个文件更好。当前只保持一个文件。
- 现在不支持并行。看看哪些地方需要加锁。