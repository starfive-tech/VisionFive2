#ifndef __TUNING_SERVICE_H__
#define __TUNING_SERVICE_H__

void tuning_serv_initial();
int tuning_serv_get_register(STCOMDDATA* pcomddata);
int tuning_serv_set_register(STCOMDDATA* pcomddata);
int tuning_serv_get_memory(STCOMDDATA* pcomddata);
int tuning_serv_set_memory(STCOMDDATA* pcomddata);
int tuning_serv_get_completed_image_info(STCOMDDATA* pcomddata);
int tuning_serv_get_completed_image(STCOMDDATA* pcomddata, void** ppmemaddr);
int tuning_serv_get_pop_image(SOCKET sock, STCOMDDATA* pcomddata);
int tuning_serv_get_isp_version(STCOMDDATA* pcomddata);
int tuning_serv_get_sensor_reg(STCOMDDATA* pcomddata);
int tuning_serv_set_sensor_reg(STCOMDDATA* pcomddata);
int tuning_serv_module_enable(STCOMDDATA* pcomddata);
int tuning_serv_is_module_enable(STCOMDDATA* pcomddata);
int tuning_serv_module_update(STCOMDDATA* pcomddata);
int tuning_serv_is_module_update(STCOMDDATA* pcomddata);
int tuning_serv_control_enable(STCOMDDATA* pcomddata);
int tuning_serv_is_control_enable(STCOMDDATA* pcomddata);
int tuning_serv_get_exposure_gain(STCOMDDATA* pcomddata);
int tuning_serv_set_exposure_gain(STCOMDDATA* pcomddata);
int tuning_serv_get_awb_gain(STCOMDDATA* pcomddata);
int tuning_serv_set_awb_gain(STCOMDDATA* pcomddata);
int tuning_serv_get_iq(STCOMDDATA* pcomddata);
int tuning_serv_set_iq(STCOMDDATA* pcomddata);
int tuning_serv_set_setting_file(STCOMDDATA* pcomddata);
int tuning_serv_set_delete_setting_file(STCOMDDATA* pcomddata);
int tuning_serv_set_restore_setting(STCOMDDATA* pcomddata);

#endif /* __TUNING_SERVICE_H__ */
