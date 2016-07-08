/*
 * COPYRIGHT 2016 SEAGATE LLC
 *
 * THIS DRAWING/DOCUMENT, ITS SPECIFICATIONS, AND THE DATA CONTAINED
 * HEREIN, ARE THE EXCLUSIVE PROPERTY OF SEAGATE TECHNOLOGY
 * LIMITED, ISSUED IN STRICT CONFIDENCE AND SHALL NOT, WITHOUT
 * THE PRIOR WRITTEN PERMISSION OF SEAGATE TECHNOLOGY LIMITED,
 * BE REPRODUCED, COPIED, OR DISCLOSED TO A THIRD PARTY, OR
 * USED FOR ANY PURPOSE WHATSOEVER, OR STORED IN A RETRIEVAL SYSTEM
 * EXCEPT AS ALLOWED BY THE TERMS OF SEAGATE LICENSES AND AGREEMENTS.
 *
 * YOU SHOULD HAVE RECEIVED A COPY OF SEAGATE'S LICENSE ALONG WITH
 * THIS RELEASE. IF NOT PLEASE CONTACT A SEAGATE REPRESENTATIVE
 * http://www.seagate.com/contact
 *
 * Original author:  Rajesh Nambiar <rajesh.nambiar@seagate.com>
 * Original author:  Kaustubh Deorukhkar   <kaustubh.deorukhkar@seagate.com>
 *
 * Original creation date: 21-Jan-2016
 */

#pragma once

#ifndef __MERO_FE_S3_SERVER_S3_PART_METADATA_H__
#define __MERO_FE_S3_SERVER_S3_PART_METADATA_H__

#include <map>
#include <string>
#include <memory>
#include <functional>

#include "s3_clovis_kvs_reader.h"
#include "s3_clovis_kvs_writer.h"
#include "s3_request_object.h"
#include "s3_object_acl.h"

enum class S3PartMetadataState {
  empty = 1,         // Initial state, no lookup done
  present,           // Part Metadata exists and was read successfully
  missing,           // Part Metadata not present in store.
  missing_partially, // Some of the Parts Metadata not present in store.
  store_created,     // Created store for Parts Metadata
  saved,             // Parts Metadata saved to store.
  deleted,           // Metadata deleted from store
  index_deleted,     // store deleted
  failed
};

class S3PartMetadata {
  // Holds system-defined metadata (creation date etc)
  // Holds user-defined metadata (names must begin with "x-amz-meta-")
  // Partially supported on need bases, some of these are placeholders
private:
  std::string account_name;
  std::string account_id;
  std::string user_name;
  std::string user_id;
  std::string bucket_name;
  std::string object_name;
  std::string upload_id;
  std::string part_number;

  std::map<std::string, std::string> system_defined_attribute;
  std::map<std::string, std::string> user_defined_attribute;

  S3ObjectACL object_ACL;
  bool is_multipart;
  std::shared_ptr<S3RequestObject> request;
  std::shared_ptr<ClovisAPI> s3_clovis_api;
  std::shared_ptr<S3ClovisKVSReader> clovis_kv_reader;
  std::shared_ptr<S3ClovisKVSWriter> clovis_kv_writer;
  bool put_metadata;

  // Used to report to caller
  std::function<void()> handler_on_success;
  std::function<void()> handler_on_failed;

  S3PartMetadataState state;

private:
  // Any validations we want to do on metadata
  void validate();
public:
  S3PartMetadata(std::shared_ptr<S3RequestObject> req, std::string uploadid,  int part_num);

  std::string get_part_index_name() {
    return "BUCKET/" + bucket_name + "/" + object_name + "/" + upload_id;
  }

  void set_content_length(std::string length);
  size_t get_content_length();
  std::string get_content_length_str();

  void set_md5(std::string md5);
  std::string get_md5();
  std::string get_object_name();
  std::string get_user_id();
  std::string get_user_name();
  std::string get_last_modified();
  std::string get_last_modified_gmt();
  std::string get_last_modified_iso();
  std::string get_storage_class();
  std::string get_upload_id();
  std::string get_part_number();

  // Load attributes
  std::string get_system_attribute(std::string key);
  void add_system_attribute(std::string key, std::string val);
  std::string get_user_defined_attribute(std::string key);
  void add_user_defined_attribute(std::string key, std::string val);

  void load(std::function<void(void)> on_success, std::function<void(void)> on_failed, int part_number);
  void load_successful();
  void load_failed();

  void save(std::function<void(void)> on_success, std::function<void(void)> on_failed);
  void create_index(std::function<void(void)> on_success, std::function<void(void)> on_failed);
  void save_part_index(std::function<void(void)> on_success, std::function<void(void)> on_failed);
  void create_part_index();
  void create_part_index_successful();
  void create_part_index_failed();
  void save_metadata();
  void save_metadata_successful();
  void save_metadata_failed();

  void remove(std::function<void(void)> on_success, std::function<void(void)> on_failed, int remove_part);
  void remove_successful();
  void remove_failed();
  void remove_index(std::function<void(void)> on_success, std::function<void(void)> on_failed);
  void remove_index_successful();
  void remove_index_failed();


  S3PartMetadataState get_state() {
    return state;
  }

  void set_state(S3PartMetadataState part_state) {
    state = part_state;
  }

  std::string to_json();

  void from_json(std::string content);
};

#endif