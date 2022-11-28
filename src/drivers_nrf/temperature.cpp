#include "drivers_nrf/temperature.h"
#include "drivers_nrf/scheduler.h"
#include "core/delegate_array.h"

#include "nrfx_temp.h"
#include "nrf_log.h"
#include "nrf_error.h"

namespace DriversNRF
{
namespace Temperature
{
    const nrfx_temp_config_t temp_config = NRFX_TEMP_DEFAULT_CONFIG;

    #define MAX_CLIENTS 2
	DelegateArray<TemperatureClientMethod, MAX_CLIENTS> clients;

    void temperatureReadyHandler(int32_t raw_measurement);
    void notifyClients(void * p_event_data, uint16_t event_size);

    void init(TemperatureInitCallback callback) {
        nrfx_temp_init(&temp_config, temperatureReadyHandler);
        ret_code_t ret = nrfx_temp_measure();
        if (ret == NRF_SUCCESS) {
            // Since reading temperature takes times and uses our interrupt handler, register a lambda
            // and we'll unregister it once we have a result.
            clients.Register((void*)callback, [] (void* the_callback, int the_temp) {
                NRF_LOG_INFO("Temperature Initialized, Temp = %d.%d C", (the_temp / 100), (the_temp % 100));
                clients.UnregisterWithToken(the_callback);
                ((TemperatureInitCallback)the_callback)(true);
            });
        } else {
            callback(false);
        }
    }

    void temperatureReadyHandler(int32_t raw_measurement) {
        int celsiusTimes100 = nrfx_temp_calculate(raw_measurement);
		for (int i = 0; i < clients.Count(); ++i) {
			clients[i].handler(clients[i].token, celsiusTimes100);
		}
    }

	/// <summary>
	/// Method used by clients to request timer callbacks when accelerometer readings are in
	/// </summary>
	void hook(TemperatureClientMethod method, void* parameter)
	{
		if (!clients.Register(parameter, method))
		{
			NRF_LOG_ERROR("Too many Temperature hooks registered.");
		}
	}

	/// <summary>
	/// Method used by clients to stop getting accelerometer reading callbacks
	/// </summary>
	void unHook(TemperatureClientMethod method)
	{
		clients.UnregisterWithHandler(method);
	}

	/// <summary>
	/// Method used by clients to stop getting accelerometer reading callbacks
	/// </summary>
	void unHookWithParam(void* param)
	{
		clients.UnregisterWithToken(param);
	}

}
}



