// Copyright 2014 Sebastian Zwierzchowski <sebastian.zwierzchowski<at>gmail<dot>com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <Controller.h>
#include <IRremote.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RGBdriver.h>
#include <RCSwitch.h>


Controller ctrl;

void setup() {
  Serial.begin(9600);
  ctrl.setupCtrl();
    
}
void loop() {
  ctrl.listen(false);
}



