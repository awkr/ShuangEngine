#include "model/Model.h"
#include "Device.h"
#include "Log.h"

Model::Model(const std::shared_ptr<Device> &device) : mDevice{device} {}

Model::~Model() { log_func; }
