/**
   USE OF THIS SOFTWARE IS GOVERNED BY THE TERMS AND CONDITIONS
   OF THE LICENSE STATEMENT AND LIMITED WARRANTY FURNISHED WITH
   THE PRODUCT.
   <p/>
   IN PARTICULAR, YOU WILL INDEMNIFY AND HOLD B2N LTD., ITS
   RELATED COMPANIES AND ITS SUPPLIERS, HARMLESS FROM AND AGAINST ANY
   CLAIMS OR LIABILITIES ARISING OUT OF THE USE, REPRODUCTION, OR
   DISTRIBUTION OF YOUR PROGRAMS, INCLUDING ANY CLAIMS OR LIABILITIES
   ARISING OUT OF OR RESULTING FROM THE USE, MODIFICATION, OR
   DISTRIBUTION OF PROGRAMS OR FILES CREATED FROM, BASED ON, AND/OR
   DERIVED FROM THIS SOURCE CODE FILE.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Over The Air Updater fuctionality

#include <esp_ota_ops.h>

esp_ota_handle_t update_handle;
uint32_t update_size;
char update_hash[41];
const esp_partition_t * update_partition;


inline void OTAInit()
{
  update_handle = 0 ;
  update_size = 0;
  update_partition = NULL;
  strcpy(update_hash, "0000000000000000000000000000000000000000");
}

//inline void OTADumpPartitions()
//{
//  size_t ul;
//  esp_partition_iterator_t _mypartiterator;
//  const esp_partition_t *_mypart;
//  ul = spi_flash_get_chip_size(); Serial.print("Flash chip size: "); Serial.println(ul);
//  Serial.println("Partiton table:");
//  _mypartiterator = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
//  if (_mypartiterator)
//  {
//    do {
//      _mypart = esp_partition_get(_mypartiterator);
//      printf("%x - %x - %x - %x - %s - %i\r\n", _mypart->type, _mypart->subtype, _mypart->address, _mypart->size, _mypart->label, _mypart->encrypted);
//    } while (_mypartiterator = esp_partition_next(_mypartiterator));
//  }
//  esp_partition_iterator_release(_mypartiterator);
//  _mypartiterator = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
//  if (_mypartiterator)
//  {
//    do {
//      _mypart = esp_partition_get(_mypartiterator);
//      printf("%x - %x - %x - %x - %s - %i\r\n", _mypart->type, _mypart->subtype, _mypart->address, _mypart->size, _mypart->label, _mypart->encrypted);
//    } while (_mypartiterator = esp_partition_next(_mypartiterator));
//  }
//}


inline void OTAEraseFull()
{
  const esp_partition_t * erase_partition = NULL;
  esp_err_t err;

  LOG64_SET(F("OTA: Erase..."));
  LOG64_NEW_LINE;

  const esp_partition_t *configured = esp_ota_get_boot_partition();
  const esp_partition_t *running = esp_ota_get_running_partition();

  if (configured != running)
  {
    LOG64_SET(F("OTA: Configured != Running["));
    LOG64_SET("0x" + String(configured->address, HEX));
    LOG64_SET("0x" + String(running->address, HEX));
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;

    return;
  }
  else
  {
    LOG64_SET(F("OTA: Running["));
    LOG64_SET("0x" + String(running->address, HEX));
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;
  }

  erase_partition = esp_ota_get_next_update_partition(NULL);
  if (erase_partition == NULL)
  {
    LOG64_SET(F("OTA: Erase address NULL"));
    LOG64_NEW_LINE;

    return;
  }
  else
  {
    LOG64_SET(F("OTA: Erase ["));
    LOG64_SET("0x" + String(erase_partition->address, HEX));
    LOG64_SET("Start : " + String(erase_partition->size));
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;
  }

  err = esp_partition_erase_range(erase_partition, 0, erase_partition->size);

  if (err != ESP_OK)
  {
    LOG64_SET(F("OTA: Erase ERR ["));
    LOG64_SET((uint32_t)err);
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;

    return;
  }
}

inline esp_err_t OTAEraseWTD()
{
  const esp_partition_t * erase_partition = NULL;
  esp_err_t err;

  LOG64_SET(F("OTA: Erase WTD..."));
  LOG64_NEW_LINE;


  erase_partition = esp_ota_get_next_update_partition(NULL);
  if (erase_partition == NULL)
  {
    LOG64_SET(F("OTA: Erase WTD address NULL"));
    LOG64_NEW_LINE;

    return ESP_ERR_OTA_BASE;
  }
  else
  {
    LOG64_SET(F("OTA: Erase WTD ["));
    LOG64_SET("0x" + String(erase_partition->address, HEX));
    LOG64_SET("Size : " + String(erase_partition->size));
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;
  }

  uint32_t blockBegin = 0;
  uint32_t blockSize = erase_partition->size;
  for (;;)
  {
    if (OTA_ERASE_SIZE > blockSize)
    {
      //last block to erase
      if (blockSize != 0)
      {
        err = esp_partition_erase_range(erase_partition, blockBegin, blockSize);
        if (err != ESP_OK)
        {
          return err;
        }
      }

      break;
    }

    err = esp_partition_erase_range(erase_partition, blockBegin, OTA_ERASE_SIZE);
    if (err != ESP_OK)
    {
      return err;
    }

    blockSize -= OTA_ERASE_SIZE;
    blockBegin += OTA_ERASE_SIZE;

    delay(64);

  }

  return ESP_OK;

}



inline void OTABegin(const char * hash)
{

  if (strcmp(update_hash, hash) == 0)
  {
    // we are in the current update cotinue
    return;
  }

  strcpy(update_hash, hash);
  update_handle = 0;
  update_partition = NULL;
  update_size = 0;

  esp_err_t err;

  LOG64_SET(F("OTA: Begin..."));
  LOG64_NEW_LINE;

  const esp_partition_t *configured = esp_ota_get_boot_partition();
  const esp_partition_t *running = esp_ota_get_running_partition();

  if (configured != running)
  {
    LOG64_SET(F("OTA: Configured != Running["));
    LOG64_SET("0x" + String(configured->address, HEX));
    LOG64_SET("0x" + String(running->address, HEX));
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;

    OTAInit();

    return;
  }
  else
  {
    LOG64_SET(F("OTA: Running["));
    LOG64_SET("0x" + String(running->address, HEX));
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;
  }


  update_partition = esp_ota_get_next_update_partition(NULL);
  if (update_partition == NULL)
  {
    LOG64_SET(F("OTA: Update address NULL"));
    LOG64_NEW_LINE;

    OTAInit();

    return;
  }
  else
  {
    LOG64_SET(F("OTA: Update["));
    LOG64_SET("0x" + String(update_partition->address, HEX));
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;
  }

  uint32_t procedureStart = millis();

  LOG64_SET(F("OTA: Erase WTD Start"));
  LOG64_NEW_LINE;

  err = OTAEraseWTD();
  if (err != ESP_OK)
  {
    LOG64_SET(F("OTA: Erase WTD ERR ["));
    LOG64_SET((uint32_t)err);
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;

    OTAInit();

    return;
  }

  LOG64_SET(F("OTA: Erase WTD Finish : "));
  LOG64_SET((uint32_t)(millis() - procedureStart));
  LOG64_NEW_LINE;

  err = esp_ota_begin(update_partition, 2048, &update_handle);

  if (err != ESP_OK)
  {
    LOG64_SET(F("OTA: Begin ERR ["));
    LOG64_SET((uint32_t)err);
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;

    OTAInit();

    return;
  }


  LOG64_SET(F("OTA: Begin Finish"));
  LOG64_NEW_LINE;

}

inline void OTAEnd()
{
  LOG64_SET(F("OTA: End..."));
  LOG64_NEW_LINE;

  esp_err_t err;

  if (( update_handle == 0) && ( update_partition == NULL))
  {
    LOG64_SET(F("OTA: End ERR [ Calling without Begin ]"));
    LOG64_NEW_LINE;

    OTAInit();

    return;
  }

  err = esp_ota_end(update_handle);
  if (err != ESP_OK)
  {
    LOG64_SET(F("OTA: End ERR ["));
    LOG64_SET((uint32_t)err);
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;

    update_handle = 0;
    update_partition = NULL;
    update_size = 0;

    return;
  }

  LOG64_SET(F("OTA:  Set Boot..."));
  LOG64_NEW_LINE;

  err = esp_ota_set_boot_partition(update_partition);
  if (err != ESP_OK)
  {
    LOG64_SET(F("OTA: Set Boot ERR ["));
    LOG64_SET((uint32_t)err);
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;

    OTAInit();

    return;
  }

  LOG64_SET(F("OTA:  Restart..."));
  LOG64_NEW_LINE;

  OTAInit();

  esp_restart();

}


inline void OTAWrite(uint8_t buf[], uint32_t buf_size, uint32_t pos)
{
  if (pos != update_size)
  {
    LOG64_SET(F("OTA: Write Wrong Pos ["));
    LOG64_SET((uint32_t)pos);
    LOG64_SET((uint32_t)update_size);
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;

    return;
  }

  //  LOG64_SET(F("OTA: Start Write Pos ["));
  //  LOG64_SET((uint32_t)pos);
  //  LOG64_SET((uint32_t)buf_size);
  //  LOG64_SET((uint32_t)update_size);
  //  LOG64_SET(F("]"));
  //  LOG64_NEW_LINE;


  esp_err_t err;

  if (( update_handle == 0) && ( update_partition == NULL))
  {
    LOG64_SET(F("OTA: Write ERR [ Calling without Begin ]"));
    LOG64_NEW_LINE;

    update_handle = 0;
    update_partition = NULL;
    update_size = 0;

    return;
  }

  err = esp_ota_write( update_handle, (const void *)buf, buf_size);
  if (err != ESP_OK)
  {
    LOG64_SET(F("OTA: Write ERR ["));
    LOG64_SET((uint32_t)err);
    LOG64_SET(F("]"));
    LOG64_NEW_LINE;

    update_handle = 0;
    update_partition = NULL;
    update_size = 0;

    return;
  }

  update_size += buf_size;

  //  LOG64_SET(F("OTA: End Write Pos ["));
  //  LOG64_SET((uint32_t)update_size);
  //  LOG64_SET(F("]"));
  //  LOG64_NEW_LINE;
}

inline uint32_t GetOTAUpdateSize()
{
  return update_size;
}
