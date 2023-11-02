# PAROL6 BOM and Sourcing Guide

## Screws

All screws are DIN 912 / ISO 4762 (hexagon socket head cap). Those can be sourced locally depending on your location.

| Size    | Quanity  |
|---------|----------|
| M4x10mm | 20       |
| M3x8mm  | 50       |
| M3x14mm | 30       |
| M3 nut  | 10       | 
| M3x25mm | 10       | 
| M2x10mm | 10       | 
| M3x16mm | 10       | 
| M3x6mm  | 10       | 
| M3x12mm | 30       |
| M3x35mm | 10       |
| M4x16mm | 30       |
| M4x14mm | 10       |
| M4x50mm | 10       |

## Steppers and Gearboxes

Steppers and gearboxes in the build are sourced from [StepperOnline](https://www.omc-stepperonline.com/) a Chinese brand with warehouses in China, US, EU, UK and Australia. High quality gears with low backlash are important for the accuracy of the robot, please use the specified gears. For the motors, if you get them from other sources, make sure they have plenty of cable length and are without any protruding connectors.

| Name in building instructions | Type                                | Quantity  |  Source
|-------------------------------|-------------------------------------|-----------|--------|
| Gearbox 20:1                  | EG17-G20 (Nema 17 20:1 low backlash)            |         2 | [StepperOnline](https://www.omc-stepperonline.com/eg-series-planetary-gearbox-gear-ratio-20-1-backlash-20-arc-min-for-nema-17-stepper-motor-eg17-g20) |
| Gearbox 10:1                  | EG17-G10 (Nema 17 10:1 low backlash)            |         1 | [StepperOnline](https://www.omc-stepperonline.com/eg-series-planetary-gearbox-gear-ratio-10-1-backlash-15-arc-min-for-nema-17-stepper-motor-eg17-g10) |
| Stepper 1 | 17HS08-1004S (Nema 17 16Ncm, 42x42x20mm) | 1 | [StepperOnline](https://www.omc-stepperonline.com/nema-17-bipolar-1-8deg-16ncm-22-6oz-in-1a-3-7v-42x42x20mm-4-wires-17hs08-1004s) |
| Stepper 2 | 17HS16-2004S1 (Nema 17 45Ncm, 42x42x40mm) | 3 | [StepperOnline](https://www.omc-stepperonline.com/nema-17-bipolar-45ncm-64oz-in-2a-42x42x40mm-4-wires-w-1m-cable-connector-17hs16-2004s1)
| Stepper 3 | 17HS24-2104S (Nema 17 65Ncm, 42x42x60mm) | 2 | [StepperOnline](https://www.omc-stepperonline.com/nema-17-bipolar-1-8deg-65ncm-92oz-in-2-1a-3-36v-42x42x60mm-4-wires-17hs24-2104s) |

## Belts and pulleys

Belts can be sourced from eBay, AliExpress or for some countries even Amazon. There is no special quality requirement. Make sure to buy the correct width. Pulleys can also be found at eBay, AliExpress or with 3d printer supplies.

Name in building instructions | Type                                  | Quantity
------------------------------|---------------------------------------|---------
Joint 1 belt                  | HTD-3M 396, 6 mm width                | 1
Joint 3 belt                  | HTD-3M 342, 6 mm width                | 1
Joint 4 belt                  | HTD-3M 201, 6 mm width                | 1
Joint 5 belt                  | HTD-3M 246, 6 mm width                | 1
J4 pulley, J5 pulley          | HTD-3M 12 tooth, 5mm bore, 10mm width | 2
J1 pulley                     | HTD-3M 15 tooth, 5mm bore, 10mm width | 1

## Shaft coupler

These can also be found as metal flange, ridig shaft copulers, motor guide etc., but most commonly as flange coupling. These can be found on Aliexpress, eBay, Amazon or sometimes with 3d printer supplies.

Name in building instructions | Type                                                 | Quantity
------------------------------|------------------------------------------------------|---------
Shaft coupler                 | flange coupling with 8 mm shaft hole, M4 screw holes | 3

## Bearings

The main bearings are important for smooth operation, therefore get only high quality bearings.

It is recommended to get the specified type from NSK, which should cost around 10-20 USD each, unless you buy fakes. They also can be found for horrendous prices as high as 160 USD equivalent, depending on your country. If you absolutely need to, you can probably replace them with 32906 and 32907 from different brands such as KBS, SKF, etc. However, this has not been tested

For the belt tension bearings, non branded cheap ones are ok. They can be found on AliExpress, eBay, Amazon in packs of 10 to 20 pieces for cheap.

For the AXK3552, make sure you buy it with top and bottom plates for a total height of 4mm. I have sometimes seen it sold without them to run on bare metal surfaces.

Name in building instructions | Type              | Quantity
------------------------------|-------------------|---------
Bearing 1 (J5)                | AXK3552 (35x52x4) | 1
Bearing 2 (J5, J4, J3)        | NSK HR32906J      | 5
Bearing 3 (J2, J1)            | NSK HR32907J      | 4
Tension bearing               | 693ZZ (3x8x4)     | 20
Tension bearing               | 623ZZ (3x10x4)    | 20
