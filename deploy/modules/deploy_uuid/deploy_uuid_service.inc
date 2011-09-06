<?php

function deploy_uuid_service_access() {
  return TRUE;
}

function deploy_uuid_service_get_key($uuid, $module){
  return deploy_uuid_get_key($uuid, $module);
}

function deploy_uuid_service_get_book($uuid){
  $nid = deploy_uuid_get_key($uuid, 'node');
  $result = db_query("SELECT * FROM {book} WHERE nid = %d", $nid);
  if ($row = db_fetch_array($result)) {
    return $row;
  }
  return array();
}