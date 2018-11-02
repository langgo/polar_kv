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

并发调用系统调用会提升效率吗？
- 猜测，在多核的情况下，应该会提升效率

## TODO

整理整个流程，并思考哪些可以聚合（聚合的意义是，并发的情况下也不一定快）

定量描述系统调用的速度

RocksDB 中描述可以并发insert SkipList ? 这个是怎么实现的

- hmap 去除8B的假设，考虑内存的释放。
- db reload 和 db 索引的存储
- 检查一下，是否有内存泄漏
- 减少内存copy。
- 怎么应用锁。
    - hash 的地方不存在 rehash，所以。取值过程中不需要锁。但是对值的处理，需要加锁。
    - 可以各个线程有自己的 write buffer，并行调用append，串行写入。
- 完善日志，以便能够获得更多的数据。

## mark

- 不进行 rehash，hash 阶段不需要锁
- 怎么保持不可变状态，这样就不需要每次重头新建索引，加快启动速度。正常的db，肯定都必须做这个事情。
- 是否考虑多个文件，如果需要整理 wal 的时候，确实多个文件更好。当前只保持一个文件。
- 现在不支持并行。看看哪些地方需要加锁。
- 去锁，异步，零拷贝，复用，批量
- 在使用 hash 作为索引的时候，O(1) ，CPU不会成为 瓶颈。（但是可能能提高性能）

## test

10w, 393M

```
cat wal.data > wal.data.a  0.00s user 0.34s system 37% cpu 0.917 total

~= 430 MB/s
当前 mac 测试:
写入 430 MB/s   x5
读取 1360 MB/s  x2
```

without hmap

```
PUT time: 1.012756 s, 9.874046 w/s 

PUT time: 1.104413 s, 9.054583 w/s
```

hmap 1024

```
PUT time: 1.212978 s, 8.244172 w/s
GET time: 2.992105 s, 3.342129 w/s

PUT time: 1.068060 s, 9.362771 w/s
GET time: 0.730888 s, 13.681984 w/s
```

hmap 4096

```
PUT time: 1.048125 s, 9.540846 w/s
GET time: 0.784400 s, 12.748598 w/s

PUT time: 0.983948 s, 10.163139 w/s
GET time: 0.868757 s, 11.510698 w/s
```
