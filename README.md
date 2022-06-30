# This is a fork of this project: https://sites.google.com/view/ysai187/home/projects/ps2ident

## PlayStation 2 Identification tool (PS2Ident)
v0.850

<details>
  <summary> <b> Introduction </b> </summary>
<p>

PS2Ident is an identification tool that allows dumps of a PlayStation 2 console's ROM chips and MECHACON NVRAM to be made.
It will also gather data from the console, for research purposes.

It has the following features:

* Dumps ROM chips (BOOT and DVD ROM) as a whole, not according to their contents (rom0, rom1, rom2 and erom)
* Displays the actual addresses for DEV1 (rom1, rom2 and erom) that are set by the ROM filesystem drivers.
* Coloured user interface that is easy to use.
* Supports dumping to memory cards and USB mass storage devices.
* Supports multi-languages, which include the 8 supported languages by the PS2
* Gathers data of all known parts of the PS2.
* Attempts to automatically match the chip/part name with the version number of the part.

*Note: on some DTL-H****** *consoles it can fail to retreive MECHACON chip version.
Note: some values are retreived incorrectly from PS3 BC consoles.
Note: it can fail on PS3 semi-BC, PS3 non-BC and PS4 in PS2 emulation mode.*

Its database, which contains the parts and mainboard data, is managed and updated with the PlayStation 2 Ident DataBase Management System (PS2IDBMS) tool. It wasn't possible to get a complete list of all PlayStation 2 models in existence. With PS2IDBMS, a spreadsheet containing all recorded models can be generated automatically.

</p>
</details>

<details>
  <summary> <b> How to use this software: </b> </summary>
<p>

Extract the whole archive (do not delete any files or folders) onto a location on a supported device.
The only supported devices are the Memory Card and USB mass storage device.

After dumping recheck if there is some "Missing" values in *_specs.txt
If you see Missing value after some chip name you can make a photo of each chip that is marked as Missing.
If you see Missing (sticker) you should provide photo of your sticker.

When the console's mainboard cannot be identified, the mainboard model will be "Missing". Open your console, and take note of the mainboard model number.
A database record file (*_database.bin) will be generated when a dump is made. No personal information is recorded in database file.
When making a submission, please provide the database record file, sticker photo and any valuable information about console.

When recording down the model of the MECHACON chip, please take note that the model number should include its firmware version as well (e.g. CXP103049-401GG instead of just CXP1013049).

Please specify the chassis model, the true model number (i.e. SCPH-39006a) and the colour of the console (if not the standard black version).

If possible, please help to complete the chip ID list as well, for any part that is not identified ("Missing") or is incorrectly identified. When doing so, please provide the full model of the part, especially for the MECHACON chip (e.g. CXP103049-401GG).

As for the chassis and EMCS ID (For Dragon-series units):
At the lower right-hand corner of the product information sticker that is either on the bottom or back of the unit, there will be something like "H FOXC".
In this example, the letter 'H' is the chassis model, while "FOXC" stands for FOXConn (the EMCS that made the console).

Notes:
* Sometimes, Sony makes hardware revisions without changing the chip implementation numbers as well, hence why chip identification may be inaccurate. The chip and version IDs are, however, accurate since they are taken directly from the hardware.
* Slimline consoles that had the SSBUS I/F Controllers integrated into the IOP will report them as version 0x31.
* Due to some SSBUS I/F controllers reporting the same version number (0x31), those will be all generalized and reported as "CXD9611". All of them are supposed to be fully compatible anyway.

</p>
</details>

<details>
  <summary> <b> Mainboards are uniquely identified by: </b> </summary>
<p>

	ROMVER string                   - ROM version string
	Model name                      - The "name" of the console's model.
	EE revision
	FPU revision                    - Revision of the EE chip's FPU.
	IOP revision
	GS revision
	MECHACON revision and region    - Revision and MagicGate region of the MECHACON
	SPU2 revision                   - Revision of the sound processor.
	SSBUS I/F Controller revision
	Model ID                        - Uniquely identifies the console's model. Even more accurately than the model name.
	Console model ID                - Another (2-byte) version of the model ID. Obtained from the i.Link ID data.
	EMCS ID                         - Uniquely identifies the Engineering, Manufacturing and Customer Services (EMCS) system that made the console.
	M Renewal Date                  - What this represents is unclear. It's something related to the MECHACON (date of firmware build or EEPROM settings update?)
	ADD0x010                        - An ID that is used by the SONY service tools to identify major revisions (which historically existed at word offset 0x10).
	BOOT + DVD ROM Checksum

If your console is not recognized, its mainboard model will appear as "Missing". While it is true that Sony consoles might be physically similar across releases from different territories, I've decided to treat each region's release as being different because I would also want to have a complete list of existing PlayStation 2 models. With such data it is possible to determine what models Sony has released in every terrirory, and at which series did they start at.

</p>
</details>

<details>
  <summary> <b> Glossary/terms: </b> </summary>
<p>

* __EE__: The Emotion Engine, which is a MIPS R5900. The "main CPU" of the PlayStation 2.
* __FPU__: The VU0, which also acts as the 2nd coprocessor to the EE.
* __IOP__: The I/O processor, which is a MIPS R3000A. The "sub-CPU" of the PlayStation 2. Although insignificant in power, it's actually in control of every part of the PS2, and provides access to the peripherals from the EE.
* __GS__: The Graphics Synthesizer (aka the "GPU" of the PS2).
* __SPU2__: The Sound Processor.
* __SSBUS I/F__: The SS-BUS InterFace (I/F) Controller, provides an interface to the SS-BUS from the DEV9 expansion device. Also known as a "SSBUS buffer" on consoles sporting an expansion-bay. Connected to the PS2 as DEV9.
* __MECHACON__: The drive MECHAnics CONtroller. Connected to the PS2 as DEV5.
* __DVP__ - I'm not sure about this one, but it's a very powerful and smart processor that is connected directly to the MECHACON.
* __Boot ROM__: The ROM chip containing the default IOP modules, the OSDSYS program, PlayStation driver and the self-test facility (On the SCPH-18000 and up). Connected to the PS2 as DEV2.
* __DVD ROM__: The ROM chip containing the DVD player, and the Chinese font on Chinese PlayStation 2 consoles. Newer consoles may have a single chip instead of a separate boot and DVD ROM chips. The SCPH-10000 and SCPH-15000, and all TEST, TOOL and PSX (DVR unit) units do not have a DVD ROM chip installed. Connected to the PS2 as DEV1.
* __NVRAM/EEPROM__: The Non-Volatile Memory storage that the MECHACON/DVP has, for storing settings (system, RTC and CD/DVD drive).
* __SPEED__: The expansion device that gets connected to the DEV9 expansion interface. May provide Ethernet support, a UART (for a dial-up modem), a DVRP, flash storage (For storing boot files), and an ATA interface.
* __SMAP__: The PlayStation 2 Ethernet interface. Connected to the SPEED device.
* Ethernet PHY: The PHYceiver device that is connected to the SMAP interface. The earliest models included a National Semiconductor DP83846A that has auto-negotiation issues with Gigabit Ethernet. Later models had a ST Microelectronics, while late slim PS2s come with a Broadcom.
* __DVRP__: __Only found on PSX (DVR unit) consoles__, the DVR Processor is a Fujitsu MB91302A that is connected to the SPEED device. It seems to be emulating the PS2's standard ATA interface and provides the DVR functionality. Contains its own firmware.
* __i.Link and USB interfaces__: The Firewire/IEEE1394/i.Link interface and USB interfaces. Their controllers are built into the IOP. Consoles that don't have an i.Link port still have the i.Link controller, but just have no physical connections to the outside world.
* __AIF: On a DTL-T10000(H)__, the CXD9566R is connected through the AIF. The AIF also provides a RTC and IDE controller.
MPU - See MRP.
* __MRP (BoardInf)__: The MRP is the interface between the PS2 and PC sides of a DTL-T10000(H). All DTL-T10000(H) units have a MPU 4.0 board as the MRP.
* __EMCS ID__: Contains the part of the IEEE1394 EUI-64 address that uniquely identifies the console, the model ID and EMCS ID.
* __i.Link ID__: Uniquely identifies the Engineering, Manufacturing and Customer Services (EMCS) system that made the console.
* __Model ID__: Uniquely identifies the console's model. Even more accurately than the model name.
* __Console ID__: Contains the EMCS ID, (another version of) model ID and serial number.
* __M Renewal Date__: What this represents is unclear. It's something related to the MECHACON (date of firmware build or EEPROM settings update?)
* __ADD0x010__: An ID that is used by the SONY service tools to identify major revisions. Originally at word 0x010 of the EEPROM, it was moved to word 0x01 for the Dragon models. It does not always correspond with the chassis models (i.e. C and D-chassis have the same ID). It can also identify the loadout of the console, like its CEX/DEX status and the OP block installed (i.e. SANYO or SONY).
* __PS1DRV Version__: The version number of the PlayStation driver. The earliest ROMs do not have PS1VER in ROM.  For Japan, only the SCPH-10000 and SCPH-15000 lack this file. "1.01" is hardcoded. For other territories (should be only the earliest SCPH-30001), "1.10" is hardcoded. The SCPH-75000 and later have a universal PS1DRV module, while earlier versions were tailored for the region of PS2 (Japan, USA, Europe, Asia or China).
* __DVD Player Version__: The version number of the built-in DVD player, if applicable. Note: Slimline consoles that had the SSBUS I/F Controller integrated into the IOP will report their SSBUS I/F controllers as version 0x31 as well. Due to some controllers reporting the same version number (0x31), they will be all identified as a "CXD9611" since they are supposed to be fully compatible anyway.

</p>
</details>

<details>
  <summary> <b> Known mechacon versions: </b> </summary>
<p>
	
| id | mecha |
| --- | --- |
| 0x0102 | CXP101064-605R |
| 0x0103 | CXP101064-602R
| 0x0106 | CXP102064-001R (Not confirmed)
| 0x0107 | CXP102064-003R
| 0x0108 | CXP102064-002R
| 0x0109 | CXP102064-751R
| 0x0200 | CXP102064-004R (Not confirmed)
| 0x0202 | CXP102064-005R
| 0x0204 | CXP102064-(1,2,3)01R
| 0x0205 | CXP102064-702R
| 0x0206 | CXP102064-(1,2,3)02R
| 0x0207 | CXP102064-703R
| 0x0208 | CXP102064-006R (Not confirmed)
| 0x0209 | CXP102064-704R (Not confirmed)
| 0x020c | CXP102064-007R/-(1,2,3)03R
| 0x020d | CXP102064-705R/-752R
| 0x020e | CXP102064-008R/-(1,2,3)04R
| 0x0300 | CXP103049-(1,2,3)01GG
| 0x0302 | CXP103049-001GG/-(1,2,3)02GG
| 0x0304 | CXP103049-401GG
| 0x0306 | CXP103049-002GG/-(1,2,3)03GG/-402GG/-501GG
| 0x0308 | CXP103049-003GG/-403GG
| 0x0500 | CXR706080-101GG
| 0x0502 | CXR706080-102GG
| 0x0504 | CXR706080-103GG
| 0x0506 | CXR706080-104GG/-106GG(MEX)
| 0x0508 | CXR706080-701GG (Not confirmed)
| 0x050a | CXR706080-702GG
| 0x050c | CXR706080-105GG/CXR706F080-1GG
| 0x050e | CXR706080-703GG
| 0x0600 | CXR716080-101GG
| 0x0602 | CXR716080-102GG
| 0x0604 | CXR716080-103GG
| 0x0606 | CXR716080-104GG
| 0x0608 | CXR716080-105GG (Not confirmed)
| 0x060a | CXR716080-106GG
| 0x060c | CXR726080-301GB
	
Mechacon chips released and 2 big families: pre-dragon (mecha ver <= 3) and dragon models (mecha ver >= 5).

Pre-dragon family chip can be recognized more detailed by looking into Mechacon region.

Region is coded into chip first symbol after dash.

For example, if mechacon version is 0x0306 and region is 0x03 then it will be chip CXP103049-303GG,

if region is Japan (0x00) then chip will be CXP103049-002GG, if Russia - CXP103049-501GG, etc.

For Dragon family, sometimes chips can be recognized only by M Renewal Date, this parametr is more accurate then chip version.

DTL dragon models report itself as odd numbered chip, but chip itself is the same.
	
***
	
</p>
</details>



<details>
  <summary> <b> Known chassis versions (this list maybe is not complete): </b> </summary>
<p>
	
| Chassis | Board | model(s) |
| ------- | ----- | -------- |
| __A__	| `GH-001`	| SCPH-10000
| __A__	| `GH-003`	| SCPH-15000
| __A+__| `GH-003`	| SCPH-18000
| __AB__| `GH-008`	| SCPH-18000
| __B__	| `GH-004`	| SCPH-30001
| __B'__| `GH-005`	| SCPH-30001
| __C__	| `GH-006`	| SCPH-30001-4
| __C'__| `GH-007`	| SCPH-30002-4
| __D__	| `GH-010`	| SCPH-30001D-SCPH-30002D
| __D'__| `GH-012`	| SCPH-30003D'-SCPH-30004D'
| __D__	| `GH-014`	| SCPH-30000-30004, SCPH-35000-35004, SCPH-30001R - SCPH-30004R
| __D__	| `GH-016`	| SCPH-30000-30004, SCPH-35000-35004, SCPH-30001R - SCPH-30004R
| __F__	| `GH-015`	| SCPH-30000, SCPH-30001R - SCPH-30007R. New CXP103049 MECHACON
| __G__	| `GH-017`	| SCPH-39000-8, SCPH-37000L, SCPH-37000B
| __G__	| `GH-019`	| SCPH-39000-8, SCPH-37000L, SCPH-37000B
| __G__	| `GH-022`	| SCPH-39000-8, SCPH-37000L, SCPH-37000B
| __H__	| `GH-023`	| SCPH-50000-11
| __I__	| `GH-026`	| SCPH-50000-11
| __J__	| `GH-029`	| SCPH-50000-11
| __K__	| `GH-032`	| SCPH-70000-12
| __K__	| `GH-035`	| SCPH-70000-12
| __L__	| `GH-037`	| SCPH-75000-12
| __L__	| `GH-040`	| SCPH-75000-12
| __L__	| `GH-041`	| SCPH-75000-12
| __M__	| `GH-051`	| SCPH-77000-12
| __M__	| `GH-052`	| SCPH-77000-12
| __N__	| `GH-061`	| SCPH-79000-12
| __N__	| `GH-062`	| SCPH-79000-12
| __P__	| `GH-070`	| SCPH-90000-12
| __P__	| `GH-071`	| SCPH-90000-12
| __R__	| `GH-072`	| SCPH-90000-12 (ROM v2.30)
| __X__	| `XPD-001`	| DESR-5000, DESR-5100, DESR-7000, DESR-7100
| __X__	| `XPD-005`	| DESR-5500, DESR-5700, DESR-7500, DESR-7700
	
</p>
</details>

<details>
  <summary> <b> Known EMCS IDs: </b> </summary>
<p>
	
|  ID | Plant |
| --- | ----- |
| `00`	| SONY EMCS (S EMCS)
| `01`	| SCPH-3900x/37000 "MADE IN CHINA" models - Assumed to all be Foxconn (FOXC)
| `02`	| SCPH-3900x/37000 SuZhou MainTek (SZMT)
| `03`	| SCPH-3900x/37000 SKZ SONY KISARAZU
| `10`	| S EMCS
| `11`	| SKD (SONY KOHDA)
| `18`	| S EMCS (PSX)
| `20`	| FOXC
| `21`	| FOXC
| `30`	| SuZhou MainTek (SZMT)
| `40`	| SONY Wuxi

</p>
</details>

<details>
  <summary> <b> List of known SSBUS I/F Controllers and their revisions: </b> </summary>
<p>

||||
| --- | --- | --- |
| CXD9546R	| 1.2	| DTL-T10000, unrefurbished.
| CXD9566R	| 2.0   |
| CXD9611R	| 3.0   |
| CXD9611AR	| 3.1   |
| CXD9611BR	| 3.1   |
| CXD9686R	| Unknown|
| CXD9686AR	| 3.1  |
| CXD9686BR	| 3.1  |
| CXD2955R	| 3.2  |

There's currently no known way to differentiate between a CXD9686 and CXD9611 on the software level,
with the exception of the CXD9611R.

</p>
</details>

<details>
  <summary> <b> Credits: </b> </summary>
<p>
	
```
	This software may have been worked on mainly by me, but I had the support of other users:
			l_Oliveira, for providing a number of test reports and giving suggestions for its design during development.
		Berion, for the icons.
		florin and the original developers of the PS2 dumper v2 tool, since I disassembled that dumper for ideas.
		Vigilante, for corrections on the CRC values for some ROMs.
		...and various sources for their dumps from the rare and obscure PS2 models!
```

</p>
</details>

<details>
  <summary> <b> Old data contributers (removed): </b> </summary>
<p>

```
	01	SCPH-70004	GH-032-11		Anonymous (K)	Old dump format
	02	SCPH-70004	GH-035-11		Anonymous (K)	Old dump format
	03	SCPH-70004	GH-035-62		Anonymous (K)	Old dump format
	04	SCPH-75003	GH-041-04		Anonymous (K)	Old dump format
	05	SCPH-75004	GH-040-02		Anonymous (K)	Old dump format
	07	SCPH-75008	GH-040-02		denben0487	Old dump format
	06	SCPH-77004	GH-052-51 (GS 1.13)	Anonymous (K)	Old dump format
	08	DESR-5500	XPD-005			vitas		Old dump format
	09	DTL-H10000S	GH-001			cde		Bad dump (hardware problem?)
```

</p>
</details>

<details>
  <summary> <b> those who volunteered to make the translations: </b> </summary>
<p>

```
	Japanese translation *: Unassigned.
	German translation (completed): Delta_force
	French translation (completed): ShaolinAssassin
	Italian translation (completed)*: master991
	Spanish translation (completed): ElPatas
	Dutch translation (completed)*: Someone who wishes to remain anonymous.
	Portuguese translation (completed): Gillbert

	And to all users who contributed test data for the early PS2Ident builds!
```

</p>
</details>
