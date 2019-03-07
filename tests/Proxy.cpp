#include "../mau.h"
#include "../Logger.h"
#include "../MauTools.h"

#include "argagg.hpp"

static logger::Channel Logger("Proxy", logger::Level::Debug);

int main(int argc, const char** argv)
{
    Logger.Info("Proxy Server");

    // Define arguments
    argagg::parser argparser{{
         {"help", {"-h", "--help"},
             "Shows this help message.", 0},
         {"port", {"-p", "--port"},
             "[default 5000] Port to listen on", 1},
         {"count", {"-c", "--count"},
             "[default 4] Number of ports in a series to forward the same way: `port`, `port+1`, ...", 1},
         {"dhost", {"-x", "--dhost"},
             "[default 127.0.0.1] Destination address", 1},
         {"dport", {"-y", "--dport"},
             "[default 6000] Destination port", 1},
         {"delay", {"-d", "--delay"},
             "[default 20] Delay of simulated link in milliseconds", 1},
         {"loss", {"-l", "--loss"},
             "[default 0] Loss rate float - set to 0 for no loss", 1},
         {"drate", {"-g", "--drate"},
             "[default 0.5] Gilbert-Elliott channel model: After a loss occurs, this is the probability that a packet makes it through", 1},
         {"bw", {"-b", "--bw"},
             "[default 20.0] Router throughput in MBPS", 1},
         {"queue", {"-q", "--queue"},
             "[default 100] Depth of router queue in milliseconds", 1},
         {"red", {"-r", "--red"},
             "[default enabled] Simulate router RED?", 0},
         {"seed", {"-s", "--seed"},
             "[default 0] RNG seed to use for randomness.  Set to 0 to use current time", 1},
         {"verbose", {"-v", "--verbose"},
             "Enable verbose mode", 0},
     }};

    // Parse arguments
    argagg::parser_results args;
    try {
        args = argparser.parse(argc, argv);
    } catch (const std::exception &e) {
        Logger.Info(argparser);
        Logger.Error("Invalid arguments: ", e.what());
        return -1;
    }

    if (args["help"]) {
        Logger.Info(argparser);
        return 0;
    }

    const int port = args["port"].as<int>(5000);
    const int count = args["count"].as<int>(4);
    const std::string dhost = args["dhost"].as<std::string>("127.0.0.1");
    const int dport = args["dport"].as<int>(6000);
    const int delay_msec = args["delay"].as<int>(20);
    const float loss_rate = args["loss"].as<float>(0.f);
    const float delivery_rate = args["drate"].as<float>(1.f);
    const float bw = args["bw"].as<float>(20.0f);
    const unsigned bw_queue = args["queue"].as<unsigned>(100);
    const bool red = args["red"].as<bool>(true);
    uint32_t rng_seed = args["seed"].as<uint32_t>(1);

    if (rng_seed == 0) {
        rng_seed = (uint32_t)mau::GetTimeUsec();
    }

    MauChannelConfig channel;
    channel.LightSpeedMsec = delay_msec;
    channel.LossRate = loss_rate;
    channel.DeliveryRate = delivery_rate;
    channel.RNGSeed = rng_seed;
    channel.Router_MBPS = bw;
    channel.Router_QueueMsec = bw_queue;
    channel.Router_RED_Enable = red;

    channel.ReorderRate = 0.f;
    channel.DuplicateRate = 0.f;
    channel.CorruptionRate = 0.f;

    std::vector<MauProxy> proxies;

    proxies.resize(count);

    for (int i = 0; i < count; ++i)
    {
        MauProxyConfig config;
        config.UDPListenPort = (uint16_t)port + i;

        MauResult createResult = mau_proxy_create(
            &config,
            &channel,
            dhost.c_str(),
            (uint16_t)dport + i,
            &proxies[i]
        );

        if (MAU_FAILED(createResult))
        {
            Logger.Error("mau_proxy_destroy failed: ", createResult);
            return -2;
        }
    }

    Logger.Debug("Press ENTER key to stop client");
    ::getchar();
    Logger.Debug("...Key press detected.  Stopping..");

    for (int i = 0; i < count; ++i)
    {
        MauResult destroyResult = mau_proxy_destroy(
            proxies[i]
        );

        if (MAU_FAILED(destroyResult))
        {
            Logger.Error("mau_proxy_destroy failed: ", destroyResult);
            return -3;
        }
    }

    return 0;
}
