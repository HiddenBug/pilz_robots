/*
 * Copyright (c) 2018 Pilz GmbH & Co. KG
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <prbt_hardware_support/pilz_modbus_client.h>

#include <prbt_hardware_support/ModbusMsgInStamped.h>
#include <prbt_hardware_support/modbus_msg_in_builder.h>
#include <prbt_hardware_support/pilz_modbus_exceptions.h>
#include <prbt_hardware_support/pilz_modbus_client_exception.h>

namespace prbt_hardware_support
{

PilzModbusClient::PilzModbusClient(ros::NodeHandle& nh,
                                   const std::vector<unsigned short>& registers_to_read,
                                   ModbusClientUniquePtr modbus_client,
                                   unsigned int response_timeout_ms,
                                   const std::string& modbus_read_topic_name,
                                   const std::string& modbus_write_service_name,
                                   double read_frequency_hz)
  : REGISTERS_TO_READ(registers_to_read)
  , RESPONSE_TIMEOUT_MS(response_timeout_ms)
  , READ_FREQUENCY_HZ(read_frequency_hz)
  , modbus_client_(std::move(modbus_client))
  , modbus_read_pub_(nh.advertise<ModbusMsgInStamped>(modbus_read_topic_name, DEFAULT_QUEUE_SIZE_MODBUS))
  , modbus_write_service_( nh.advertiseService(modbus_write_service_name,
                                               &PilzModbusClient::modbus_write_service_cb,
                                               this) )
{
  ROS_ERROR_STREAM("PilzModbusClient Constructor");
}


bool PilzModbusClient::init(const char* ip, unsigned int port,
                            unsigned int retries, ros::Duration timeout)
{
  for(size_t retry_n = 0; retry_n < retries; ++retry_n)
  {
    if(init(ip, port))
    {
      return true;
    }

    ROS_ERROR_STREAM("Connection to " << ip << ":" << port << " failed. Try(" << retry_n+1 << "/" << retries << ")");

    if(!ros::ok())
    {
      break; // LCOV_EXCL_LINE Simple functionality but hard to test
    }
    timeout.sleep();
  }

  return false;
}


bool PilzModbusClient::init(const char* ip, unsigned int port)
{
  State expectedState {State::not_initialized};
  if (!state_.compare_exchange_strong(expectedState, State::initializing))
  {
    ROS_ERROR_STREAM("(A) Modbus-client not in correct state: " << state_ << "expected:" << expectedState);
    state_ = State::not_initialized;
    return false;
  }

  if (!modbus_client_->init(ip, port))
  {
    ROS_ERROR_STREAM("Init failed !");
    state_ = State::not_initialized;
    return false;
  }

  modbus_client_->setResponseTimeoutInMs(RESPONSE_TIMEOUT_MS);

  state_ = State::initialized;
  ROS_ERROR_STREAM("Connection to " << ip << ":" << port << " established");
  return true;
}

void PilzModbusClient::sendDisconnectMsg()
{
  ModbusMsgInStamped msg;
  msg.disconnect.data = true;
  msg.header.stamp = ros::Time::now();
  modbus_read_pub_.publish(msg);
  ros::spinOnce();
}

void PilzModbusClient::run()
{
  ROS_ERROR_STREAM("PilzModbusClient::run");
  State expectedState {State::initialized};
  if (!state_.compare_exchange_strong(expectedState, State::running))
  {
    ROS_ERROR_STREAM("(B) Modbus-client not in correct state: " << state_ << "expected:" << expectedState);
    throw PilzModbusClientException("Modbus-client not in correct state.");
  }

  RegCont holding_register;
  RegCont last_holding_register;
  ros::Time last_update {ros::Time::now()};
  state_ = State::running;
  ros::Rate rate(READ_FREQUENCY_HZ);
  while ( ros::ok() && !stop_run_.load() )
  {
    // Work with local copy of buffer to ensure that the service callback
    // function does not become blocked
    boost::optional<ModbusRegisterBlock> write_reg_bock {boost::none};
    {
      std::lock_guard<std::mutex> lock(write_reg_blocks_mutex_);
      if (!write_reg_blocks_.empty())
      {
        write_reg_bock = write_reg_blocks_.front();
        // Mark data as send/processed, by "deleting" them from memory
        write_reg_blocks_.pop();
      }
    }

    std::vector<std::vector<unsigned short>> blocks;
    split_into_blocks(blocks, REGISTERS_TO_READ);
    for(auto &reg : REGISTERS_TO_READ)
      ROS_ERROR("- %d", reg);

    unsigned short index_of_first_register = *std::min_element(REGISTERS_TO_READ.begin(), REGISTERS_TO_READ.end());
    int num_registers = *std::max_element(REGISTERS_TO_READ.begin(), REGISTERS_TO_READ.end()) - index_of_first_register;
    holding_register = RegCont(num_registers, 0);
    ROS_ERROR("holding_register = RegCont(num_registers, 0);");

    ROS_ERROR("blocks.size() %d", blocks.size());
    ROS_ERROR("blocks[0].size() %d", blocks[0].size());
    for(auto &block : blocks){
      ROS_ERROR("block.size() %d", block.size());
      unsigned short index_of_first_register_block = *(block.begin());
      unsigned long num_registers_block = block.size();
      RegCont block_holding_register;
      try
      {
        ROS_ERROR("index_of_first_register_block: %d", index_of_first_register_block);
        ROS_ERROR("num_registers_block: %d", num_registers_block);
        if (write_reg_bock)
        {
          block_holding_register = modbus_client_->writeReadHoldingRegister(static_cast<int>(write_reg_bock->start_idx),
                                                                      write_reg_bock->values,
                                                                      static_cast<int>(index_of_first_register_block),
                                                                      static_cast<int>(num_registers_block));
          // write only once:
          write_reg_bock = boost::none;
        }
        else
        {
          block_holding_register = modbus_client_->readHoldingRegister(static_cast<int>(index_of_first_register_block), static_cast<int>(num_registers_block));
          ROS_ERROR("modbus_client_->readHoldingRegister");
        }
        for(uint i = 0; i < num_registers_block; i++){
          ROS_ERROR("i: %d, index_of_first_register: %d, block_holding_register[i] %d",
                    i,
                    index_of_first_register,
                    block_holding_register[i]);
          holding_register[i+index_of_first_register] = block_holding_register[i];
        }
      }
      catch(ModbusExceptionDisconnect &e)
      {
        ROS_ERROR_STREAM(e.what());
        sendDisconnectMsg();
        break;
      }
    }
    ROS_ERROR("for(auto &block : blocks){");

    ModbusMsgInStampedPtr msg {
      ModbusMsgInBuilder::createDefaultModbusMsgIn(index_of_first_register, holding_register)
    };

    // Publish the received data into ROS
    if(holding_register != last_holding_register)
    {
      ROS_DEBUG_STREAM("Sending new ROS-message.");
      msg->header.stamp = ros::Time::now();
      last_update = msg->header.stamp;
      last_holding_register = holding_register;
    }
    else
    {
      msg->header.stamp = last_update;
    }
    modbus_read_pub_.publish(msg);

    ros::spinOnce();
    rate.sleep();
  }

  stop_run_ = false;
  state_ = State::not_initialized;
}

void PilzModbusClient::split_into_blocks(std::vector<std::vector<unsigned short>> &out, const std::vector<unsigned short> &in){
  unsigned short prev{0};
  std::vector<unsigned short> current_block;
  for (auto & reg : in){
    ROS_ERROR("reg: %d, prev: %d", reg, prev);
    if(reg <= prev){
      throw PilzModbusClientException("List must be sorted.");
    }
    else if(reg == prev + 1) {
      current_block.push_back(reg);
    }
    else { // *it >= prev + 1
      std::vector<unsigned short> to_out(current_block);
      if(to_out.size() > 0)
        out.push_back(to_out);
      current_block.clear();
      current_block.push_back(reg);
    }
    prev = reg;
  }
  std::vector<unsigned short> to_out(current_block);
  if(to_out.size() > 0)
    out.push_back(to_out);
}

}  // namespace prbt_hardware_support
