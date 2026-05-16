const config_module = require('./config');
const Redis = require('ioredis');

// 创建 Redis 客户端实例
const RedisCli = new Redis({
    host: config_module.redis_host,
    port: config_module.redis_port,
    password: config_module.redis_passwd
});

/**
 * 处理 Redis 连接错误事件
 * 当 Redis 连接发生错误时，输出错误信息并关闭连接
 */
RedisCli.on('error', function (err) {
    console.log("RedisCli connect error");
    RedisCli.quit();
});

/**
 * 根据key获取Redis中的值
 * @param {*} key 
 * @returns 
 */
async function GetResis(key) {
    try {
        const result = await RedisCli.get(key); // 使用 await 等待 Redis 的 get 操作完成
        if (result === null) {
            console.log(`result:${result} This key cannot be found...`);
            return null; // 返回 null 或其他适当的值以表示键不存在
        }
        console.log(`result:${result} Get key success!...`);
        return result; // 返回获取到的值
    }
    catch (error) {
        console.error('GetRedis error is', error);
        return null; // 返回 null 或其他适当的值以表示获取失败
    }

}

/**
 * 根据key查询Redis中是否存在该键
 * @param {*} key 
 * @returns 
 */
async function QueryRedis(key) {
    try {
        const result = await RedisCli.exists(key); // 使用 await 等待 Redis 的 exists 操作完成
        if (result === 0) {
            console.log(`result:${result} This key is null...`);
            return null; // 返回 false 或其他适当的值以表示键不存在
        }
        console.log(`Result:${result} With this value!...`);
        return result; // 返回 true 或其他适当的值以表示键存在
    }
    catch (error) {
        console.error('QueryRedis error is', error);
        return null; // 返回 null 或其他适当的值以表示查询失败
    }

}

/**
 * 设置 Redis 键值对并指定过期时间
 * @param {*} key 
 * @param {*} value 
 * @param {*} exptime 
 * @returns 
 */
async function SetRedisExpire(key, value, exptime) {
    try {
        // 设置键和值
        await RedisCli.set(key, value);
        // 设置键的过期时间（单位：秒）
        await RedisCli.expire(key, exptime);
        return true; // 返回 true 或其他适当的值以表示设置成功

    } catch (error) {
        console.error('SetRedisExpire error is', error);
        return null; // 返回 null 或其他适当的值以表示设置失败
    }

}

/**
 * 关闭 Redis 连接
 */
function Quit() {
    RedisCli.quit(); // 关闭 Redis 连接
}

module.exports = {
    GetResis,
    QueryRedis,
    SetRedisExpire,
    Quit
};