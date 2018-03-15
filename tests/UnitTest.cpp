#include "../mau.h"
#include "../Logger.h"

static logger::Channel Logger("Test", logger::Level::Debug);

// If this is defined, it will run a proxy until ENTER
//#define MAU_SIMPLE_PROXY

bool TestAPI()
{
    MauChannelConfig channel;
    channel.LightSpeedMsec = 100;
    channel.LossRate = 0.05f;
    channel.RNGSeed = 1;
    MauProxyConfig config;
    config.UDPListenPort = 10200;
    const char* hostname = "localhost";
    uint16_t port = 5060;
    MauProxy proxy;
    MauResult createResult = mau_proxy_create(
        &config,
        &channel,
        hostname,
        port,
        &proxy
    );
    if (MAU_FAILED(createResult))
    {
        Logger.Error("mau_proxy_destroy failed: ", createResult);
        return false;
    }

#ifdef MAU_SIMPLE_PROXY
    Logger.Debug("Press ENTER key to stop client");
    ::getchar();
    Logger.Debug("...Key press detected.  Stopping..");
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Example reconfigure route settings during use
    mau_proxy_config(
        proxy,
        &channel
    );

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
#endif

    MauResult destroyResult = mau_proxy_destroy(
        proxy
    );
    if (MAU_FAILED(createResult))
    {
        Logger.Error("mau_proxy_destroy failed: ", destroyResult);
        return false;
    }

    return true;
}

int main()
{
    Logger.Info("Basic API test running for 2 seconds");

    const bool result = TestAPI();

    Logger.Info("Test returned result = ", result);

    return result ? 0 : -1;
}
