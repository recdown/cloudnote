import { Injectable } from '@nestjs/common';
import * as crypto from 'node:crypto';

@Injectable()
export class CryptoService {
  /**
   * 为密码哈希生成随机盐
   * @returns {string} 盐
   */
  generateSalt() {
    return crypto.randomBytes(16).toString('hex');
  }

  /**
   * 使用盐哈希密码
   * @param {string} password 密码
   * @param {string} salt 盐
   * @returns {string} 哈希后的密码
   */

  hashPassword(password: string, salt: string) {
    return crypto
      .pbkdf2Sync(password, salt, 1000, 64, 'sha512')
      .toString('hex');
  }

  /**
   * 验证密码是否正确
   * @param {string} password 密码
   * @param {string} salt 盐
   * @param {string} hash 哈希后的密码
   * @returns {boolean} 密码是否正确
   */
  verityPassword(password: string, salt: string, hash: string) {
    return this.hashPassword(password, salt) === hash;
  }
}
