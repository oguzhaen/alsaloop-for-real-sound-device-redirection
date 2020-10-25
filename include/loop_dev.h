/**
 * @file loop_dev.h
 * @author Oguzhan MUTLU (oguzhan.mutlu@daiichi.com)
 * @brief Bu sınıf Alsa Loopback device ile source yönetimi için gerekli
 *        fonksiyonları içerir.
 * @version 0.1
 * @date 2019-11-22
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef LOOP_DEV_H
#define LOOP_DEV_H

#include <string>
#include <vector>

#include "alsaloop.h"

/**
 * @brief Loop cihazaları tutmak için sınıf içerisinde kullanılan yapı.
 * 
 */
struct loopbackDev;

/**
 * @brief Alsa loop cihazları ile gerçek ses cihazlarını bağlayan sınıftır.
 * 
 *        1 Loop cihazı yalnızca 1 tane gerçek ses cihazına bağlanabilir.
 *        Aynı zamanda 1 gerçek ses cihazı da tek bir loop cihaza bağlanabilir.
 * 
 *        Bir Loop cihazı 2 alt cihazdan oluşur. Capture alt cihazı
 *        gerçek cihaz ile bağlantının kurulduğu cihazdır. Playback alt cihazı
 *        ise client taraftaki streame verilecek olan captureda kaydedilen
 *        ses verisinin aktarılacağı cihazdır.
 * 
 */
class LoopDev
{
private:

    loopbackDev * loopDev;
    AlsaLoop * alsaLoop;
    static std::vector<std::string> realDevs;
    bool isLoopDevConnected;
    std::string connectedRealDev;

public:

    LoopDev();
    virtual ~LoopDev();

    /**
     * @brief Verilen gerçek cihaz ismi ile 
     *        uygun olan loop cihazlarından biri bağlanır.
     * 
     * @param realDev 
     * @return int : 0 - başarılı
     *             : 1 - gerçek cihaz meşgul
     *             : 2 - loop cihaz başka bir gerçek cihaza bağlı
     *             : -1 - Bağlantı kurulamadı.(Alsa hatası)
     */
    int connect(const std::string &realDev);

    /**
     * @brief Gerçek cihaz ile loop cihaz bağlantısı kopartılır
     * 
     * @return int : 0 - başarılı
     *             : diğer - başarısız
     */
    int disconnect();

    /**
     * @brief Loop cihazının capture alt cihazının ismi döner.
     * 
     * @return std::string 
     */
    std::string getCaptureLoopDevName() const;

    /**
     * @brief Loop cihazının playback alt cihazının ismi döner.
     * 
     * @return std::string 
     */
    std::string getPlaybackLoopDevName() const;

    /**
     * @brief Bu Loop cihazının bağlantı kurduğu gerçek cihaz adı döner.
     *        Bu fonksiyon connection varsa değer döndürür. 
     *        Bu nedenle geri dönüş değeri kontrol edildikten sonra 
     *        string okunmalıdır.
     * 
     * @param realDevName 
     * @return true 
     * @return false realdev tanımlı değil
     */
    bool getRealDevName(std::string &realDevName) const;

    /**
     * @brief Nesne bağlantılı mı değil mi döner.
     * 
     * @return true 
     * @return false 
     */
    bool isLoopConnected() const;

    /**
     * @brief Verilen real device'in herhangi bir loop device
     *        ile bağlantısı var mı kontrol eder.
     * 
     * @param realDev 
     * @return true connection var.
     * @return false connection yok.
     */
    bool isRealConnected(const std::string &realDev) const;
};

#endif /*LOOP_DEV_H*/
