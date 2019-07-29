/*
 * Copyright (c) 2019 Pilz GmbH & Co. KG
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

#ifndef MODBUS_API_SPEC_H
#define MODBUS_API_SPEC_H

#include <algorithm>
#include <initializer_list>
#include <map>
#include <string>
#include <utility>

#include <ros/ros.h>
#include <xmlrpcpp/XmlRpc.h>

namespace prbt_hardware_support
{

namespace modbus_api_spec
{
  static const std::string STO {"STO"};
  static const std::string VERSION {"VERSION"};
  static const std::string BRAKETEST_REQUEST {"BRAKETEST_REQUEST"};
  static const std::string OPERATION_MODE {"OPERATION_MODE"};
}

/**
 * @brief Expection thrown by prbt_hardware_support::ModbusApiSpec
 */
class ModbusApiSpecException : public std::runtime_error
{
public:
  ModbusApiSpecException( const std::string& what_arg ):
    std::runtime_error(what_arg)
  {
  }
};

/**
 * @brief Specifies the meaning of the holding registers.
 *
 * @remark this class is templated for easier mocking. However for usability
 * it can be used by ModbusApiSpec
 */
template <class T = ros::NodeHandle>
class ModbusApiSpecTemplated
{
public:

  ModbusApiSpecTemplated(std::initializer_list< std::pair<std::string, unsigned int> > reg_list)
  {
    for(auto entry : reg_list){
      setRegisterDefinition(entry.first, entry.second);
    }
  }

  /**
   * @brief Construct a new Modbus Api Spec Templated object.
   *
   * The parameters are expected to be provided as
   * @code
   * /[nodehandle_namespace]/api_spec/[key1]
   * /[nodehandle_namespace]/api_spec/[key2]
   * ...
   * @endcode
   * with the values beeing of type <b>int</b>.
   *
   * @param nh NodeHandle to read the parameters from
   */
  ModbusApiSpecTemplated(T &nh)
  {
    XmlRpc::XmlRpcValue rpc;
    if (!nh.getParam("api_spec/", rpc))
    {
      throw ModbusApiSpecException("No api specified. (Expected at " + nh.getNamespace() + "/api_spec/");
    }

    for (auto rpci = rpc.begin(); rpci != rpc.end(); ++rpci)
    {
      int value = rpci->second;
      setRegisterDefinition(rpci->first.c_str(), static_cast<unsigned int>(value));
    }
  }

  inline bool hasRegisterDefinition(const std::string &key) const
  {
    return register_mapping_.find(key) != register_mapping_.end();
  }

  inline void setRegisterDefinition(const std::string &key, unsigned int value)
  {
    register_mapping_[key] = value;
  }

  inline unsigned int getRegisterDefinition(const std::string &key) const
  {
    try
    {
      return register_mapping_.at(key);
    }
    catch(const std::out_of_range& e)
    {
      throw ModbusApiSpecException(e.what());
    }
  }
  inline unsigned int getMinRegisterDefinition() const
  {
    if (register_mapping_.empty())
    {
      throw ModbusApiSpecException("Cannot read values. Api spec is empty.");
    }

    typedef std::pair<std::string, unsigned int> RegisterMappingEntry;
    // The following is excluded because lambda functions are not marked properly with gcc-7
    // see https://github.com/gcc-mirror/gcc/commit/7de708f
    // LCOV_EXCL_START
    auto it = std::min_element(register_mapping_.begin(), register_mapping_.end(),
                               [] (const RegisterMappingEntry& a, const RegisterMappingEntry& b)
                               { return a.second < b.second; });
    // LCVO EXCL_STOP
    return it->second;
  }

  inline unsigned int getMaxRegisterDefinition() const
  {
    if (register_mapping_.empty())
    {
      throw ModbusApiSpecException("Cannot read values. Api spec is empty.");
    }

    typedef std::pair<std::string, unsigned int> RegisterMappingEntry;
    // The following is excluded because lambda functions are not marked properly with gcc-7
    // see https://github.com/gcc-mirror/gcc/commit/7de708f
    // LCOV_EXCL_START
    auto it = std::max_element(register_mapping_.begin(), register_mapping_.end(),
                               [] (const RegisterMappingEntry& a, const RegisterMappingEntry& b)
                               { return a.second < b.second; });
    // LCVO EXCL_STOP
    return it->second;
  }

private:
  std::map<std::string, unsigned int> register_mapping_;
};

//! Simple typedef for class like usage
typedef ModbusApiSpecTemplated<> ModbusApiSpec;

} // namespace prbt_hardware_support
#endif // MODBUS_API_SPEC_H