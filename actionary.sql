DROP TABLE IF EXISTS `act_prop`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `act_prop` (
  `act_id` int(10) unsigned NOT NULL,
  `table_id` int(10) unsigned NOT NULL,
  `prop_value` int(11) NOT NULL,
  PRIMARY KEY (`act_id`,`table_id`,`prop_value`),
  KEY `tab_idx` (`table_id`),
  CONSTRAINT `act` FOREIGN KEY (`act_id`) REFERENCES `action` (`act_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `tab` FOREIGN KEY (`table_id`) REFERENCES `property_type` (`prop_id`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `act_prop`
--

LOCK TABLES `act_prop` WRITE;
/*!40000 ALTER TABLE `act_prop` DISABLE KEYS */;
INSERT INTO `act_prop` VALUES (2,16,1),(2,16,2),(2,16,3),(2,16,4),(5,16,3),(5,16,4);
/*!40000 ALTER TABLE `act_prop` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `action`
--

DROP TABLE IF EXISTS `action`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `action` (
  `act_id` int(10) unsigned NOT NULL,
  `act_name` varchar(40) NOT NULL DEFAULT '',
  `act_appl_cond` varchar(40) DEFAULT NULL,
  `act_prep_spec` varchar(40) DEFAULT NULL,
  `act_exec_steps` varchar(40) DEFAULT NULL,
  `act_term_cond` varchar(40) DEFAULT NULL,
  `act_purpose_achieve` varchar(60) DEFAULT NULL,
  `act_dur_time_id` int(11) DEFAULT '-1',
  `act_site_type_id` int(11) DEFAULT '-1',
  `wordnet_sense` smallint(6) DEFAULT '-1',
  PRIMARY KEY (`act_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `action`
--

LOCK TABLES `action` WRITE;
/*!40000 ALTER TABLE `action` DISABLE KEYS */;
INSERT INTO `action` VALUES (0,'ROOT',NULL,NULL,NULL,NULL,NULL,-1,-1,-1),(1,'Act',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(2,'Interact',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(3,'Communicate','Gesticulate.py','Gesticulate.py','Gesticulate.py','Gesticulate.py',NULL,-1,-1,2),(4,'Inform','Inform.py','Inform.py','Inform.py','Inform.py',NULL,-1,-1,1),(5,'Talk','Talk.py','Talk.py','Talk.py','Talk.py',NULL,-1,-1,2),(6,'Gesticulate',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(7,'Nod',NULL,NULL,NULL,NULL,NULL,-1,-1,2),(8,'Indicate',NULL,NULL,NULL,NULL,NULL,-1,-1,2),(9,'Speak','Speak.py','Speak.py','Speak.py','Speak.py',NULL,-1,-1,-1),(10,'Shake',NULL,NULL,NULL,NULL,NULL,-1,-1,9),(11,'PointAt',NULL,NULL,NULL,NULL,NULL,-1,-1,-1),(12,'Move',NULL,NULL,NULL,NULL,NULL,-1,-1,3),(13,'Shake','Gesticulate.py','Gesticulate.py','Gesticulate.py','Gesticulate.py',NULL,-1,-1,1),(14,'Jump',NULL,NULL,NULL,NULL,NULL,-1,-1,8),(15,'Jiggle',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(16,'Wag','Wag.py','Wag.py','Wag.py','Wag.py',NULL,-1,-1,1),(17,'Waggle',NULL,NULL,NULL,NULL,NULL,-1,-1,-1),(18,'Change',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(19,'Wet','Wet.py','Wet.py','Wet.py','Wet.py',NULL,-1,-1,1),(20,'Clean',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(21,'Better',NULL,NULL,NULL,NULL,NULL,-1,-1,2),(22,'Water',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(23,'Wash','Wash.py','Wash.py','Wash.py','Wash.py',NULL,-1,-1,3),(24,'Fancify',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(25,'Groom',NULL,NULL,NULL,NULL,NULL,-1,-1,3),(26,'Cleanse',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(27,'Wash','Wash.py','Wash.py','Wash.py','Wash.py',NULL,-1,-1,2),(28,'WashSelf',NULL,NULL,NULL,NULL,NULL,-1,-1,-1),(29,'Move',NULL,NULL,NULL,NULL,NULL,-1,-1,2),(30,'Propel',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(31,'Put','Put.py','Put.py','Put.py','Put.py',NULL,-1,-1,1),(32,'Engage',NULL,NULL,NULL,NULL,NULL,-1,-1,10),(33,'Throw',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(34,'SetDown',NULL,NULL,NULL,NULL,NULL,-1,-1,4),(35,'Throw',NULL,NULL,NULL,NULL,NULL,-1,-1,6),(36,'Fling',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(37,'PutDown',NULL,NULL,NULL,NULL,NULL,-1,-1,-1),(38,'SwitchOn',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(39,'Flip','Flip.py','Flip.py','Flip.py','Flip.py',NULL,-1,-1,6),(40,'TurnOn','TurnOn.py','TurnOn.py','TurnOn.py','TurnOn.py',NULL,-1,-1,-1),(41,'Toss',NULL,NULL,NULL,NULL,NULL,-1,-1,-1),(42,'Travel','Travel.py','Travel.py','Travel.py','Travel.py',NULL,-1,-1,1),(43,'TravelRapidly',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(44,'Step',NULL,NULL,NULL,NULL,NULL,-1,-1,2),(45,'Walk',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(46,'Run',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(47,'Trot',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(48,'Jog',NULL,NULL,NULL,NULL,NULL,-1,-1,-1),(49,'Change',NULL,NULL,NULL,NULL,NULL,-1,-1,2),(50,'ChangePosture',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(51,'SitDown','SitDown.py','SitDown.py','SitDown.py','SitDown.py',NULL,-1,-1,1),(52,'Sit',NULL,NULL,NULL,NULL,NULL,-1,-1,-1),(53,'Get','Get.py','Get.py','Get.py','Get.py',NULL,-1,-1,1),(54,'Collect',NULL,NULL,NULL,NULL,NULL,-1,-1,5),(55,'Catch',NULL,NULL,NULL,NULL,NULL,-1,-1,10),(56,'PickUp',NULL,NULL,NULL,NULL,NULL,-1,-1,-1),(57,'Make','Make.py','Make.py','Make.py','Make.py',NULL,-1,-1,3),(58,'LayDown',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(59,'CreateFromRawMaterial',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(60,'Cook','Cook.py','Cook.py','Cook.py','Cook.py',NULL,-1,-1,1),(61,'Compete',NULL,NULL,NULL,NULL,NULL,-1,-1,1),(62,'Play','Play.py','Play.py','Play.py','Play.py',NULL,-1,-1,1),(63,'Perceive','Perceive.py','Perceive.py','Perceive.py','Perceive.py',NULL,-1,-1,1),(64,'Touch',NULL,NULL,NULL,NULL,NULL,-1,-1,2),(65,'Look','Perceive.py','Perceive.py','Perceive.py','Perceive.py',NULL,-1,-1,1),(66,'Gaze',NULL,NULL,NULL,NULL,NULL,-1,-1,1);
/*!40000 ALTER TABLE `action` ENABLE KEYS */;
UNLOCK TABLES;


--
-- Table structure for table `action_parent`
--

DROP TABLE IF EXISTS `action_parent`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `action_parent` (
  `act_id` int(10) unsigned NOT NULL,
  `parent_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`act_id`,`parent_id`),
  KEY `act_parent_parent_id_idx` (`parent_id`),
  CONSTRAINT `act_parent_act_id` FOREIGN KEY (`act_id`) REFERENCES `action` (`act_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `act_parent_parent_id` FOREIGN KEY (`parent_id`) REFERENCES `action` (`act_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `action_parent`
--

LOCK TABLES `action_parent` WRITE;
/*!40000 ALTER TABLE `action_parent` DISABLE KEYS */;
INSERT INTO `action_parent` VALUES (1,0),(12,0),(18,0),(29,0),(42,0),(49,0),(53,0),(57,0),(61,0),(63,0),(65,0),(2,1),(3,2),(4,3),(5,3),(6,3),(7,3),(8,4),(9,5),(10,6),(11,8),(13,12),(14,12),(15,13),(16,15),(17,16),(19,18),(20,18),(21,18),(22,19),(23,20),(24,21),(25,24),(26,25),(27,26),(28,27),(30,29),(31,29),(32,29),(33,30),(34,31),(35,32),(36,33),(37,34),(38,35),(39,36),(40,38),(41,39),(43,42),(44,42),(45,42),(46,43),(47,46),(48,47),(50,49),(51,50),(52,51),(54,53),(55,53),(56,54),(58,57),(59,57),(60,59),(62,61),(64,63),(66,65);
/*!40000 ALTER TABLE `action_parent` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adverb_exp`
--

DROP TABLE IF EXISTS `adverb_exp`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adverb_exp` (
  `act_id` int(10) unsigned NOT NULL DEFAULT '0',
  `adverb_name` varchar(20) DEFAULT NULL,
  `adverb_mod_name` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`act_id`),
  CONSTRAINT `adverb_action` FOREIGN KEY (`act_id`) REFERENCES `action` (`act_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adverb_exp`
--

LOCK TABLES `adverb_exp` WRITE;
/*!40000 ALTER TABLE `adverb_exp` DISABLE KEYS */;
/*!40000 ALTER TABLE `adverb_exp` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `obj_act`
--

DROP TABLE IF EXISTS `obj_act`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `obj_act` (
  `obj_id` int(10) unsigned NOT NULL DEFAULT '0',
  `act_id` int(10) unsigned NOT NULL DEFAULT '0',
  `obj_num` smallint(5) unsigned NOT NULL,
  PRIMARY KEY (`obj_id`,`act_id`),
  KEY `Action` (`act_id`),
  CONSTRAINT `act_action` FOREIGN KEY (`act_id`) REFERENCES `action` (`act_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `act_object` FOREIGN KEY (`obj_id`) REFERENCES `object` (`obj_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `obj_act`
--

LOCK TABLES `obj_act` WRITE;
/*!40000 ALTER TABLE `obj_act` DISABLE KEYS */;
INSERT INTO `obj_act` VALUES (1,6,0),(1,57,0),(1,63,0),(1,66,0),(2,19,0),(2,23,0),(2,27,0),(2,39,2),(2,57,1),(2,60,1),(28,31,0),(28,39,1),(28,42,1),(28,45,1),(28,47,1),(28,53,0),(28,60,0),(33,7,0),(33,16,0),(33,31,1),(33,39,0),(33,42,0),(33,45,0),(33,47,0);
/*!40000 ALTER TABLE `obj_act` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `obj_capable`
--

DROP TABLE IF EXISTS `obj_capable`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `obj_capable` (
  `obj_id` int(10) unsigned NOT NULL DEFAULT '0',
  `action_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`obj_id`,`action_id`) USING BTREE,
  KEY `Action` (`action_id`),
  CONSTRAINT `capable_action` FOREIGN KEY (`action_id`) REFERENCES `action` (`act_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `capable_object` FOREIGN KEY (`obj_id`) REFERENCES `object` (`obj_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `obj_capable`
--

LOCK TABLES `obj_capable` WRITE;
/*!40000 ALTER TABLE `obj_capable` DISABLE KEYS */;
/*!40000 ALTER TABLE `obj_capable` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `obj_prop`
--

DROP TABLE IF EXISTS `obj_prop`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `obj_prop` (
  `obj_id` int(10) unsigned NOT NULL,
  `table_id` int(10) unsigned NOT NULL,
  `prop_value` int(11) NOT NULL,
  PRIMARY KEY (`prop_value`,`table_id`,`obj_id`),
  KEY `obj_idx` (`obj_id`),
  KEY `tab_idx` (`table_id`),
  CONSTRAINT `prop_obj` FOREIGN KEY (`obj_id`) REFERENCES `object` (`obj_id`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  CONSTRAINT `prop_table` FOREIGN KEY (`table_id`) REFERENCES `property_type` (`prop_id`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `obj_prop`
--

LOCK TABLES `obj_prop` WRITE;
/*!40000 ALTER TABLE `obj_prop` DISABLE KEYS */;
/*!40000 ALTER TABLE `obj_prop` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `obj_status`
--

DROP TABLE IF EXISTS `obj_status`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `obj_status` (
  `id_value` int(10) unsigned NOT NULL DEFAULT '0',
  `name_value` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`id_value`),
  UNIQUE KEY `id_value_UNIQUE` (`id_value`),
  UNIQUE KEY `name_value_UNIQUE` (`name_value`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `obj_status`
--

LOCK TABLES `obj_status` WRITE;
/*!40000 ALTER TABLE `obj_status` DISABLE KEYS */;
INSERT INTO `obj_status` VALUES (4,'BROKEN'),(8,'CLEARED'),(5,'EMPTY'),(6,'FULL'),(2,'IDLE'),(9,'NUM_STATUS'),(0,'OFF'),(1,'ON'),(3,'OPERATING'),(7,'UNCLEARED');
/*!40000 ALTER TABLE `obj_status` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `object`
--

DROP TABLE IF EXISTS `object`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `object` (
  `obj_id` int(10) unsigned NOT NULL,
  `obj_name` varchar(40) NOT NULL,
  `is_agent` tinyint(1) DEFAULT '0',
  `parent_id` int(11) NOT NULL,
  `wordnet_sense` smallint(6) DEFAULT '-1',
  PRIMARY KEY (`obj_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `object`
--

LOCK TABLES `object` WRITE;
/*!40000 ALTER TABLE `object` DISABLE KEYS */;
INSERT INTO `object` VALUES (1,'Entity',0,-1,1),(2,'PhysicalEntity',0,1,1),(3,'Object',0,2,1),(4,'Matter',0,2,3),(5,'CausalAgent',1,2,1),(6,'Whole',0,3,2),(7,'Substance',0,4,1),(8,'Person',1,5,1),(9,'Artifact',0,6,1),(10,'LivingThing',0,6,1),(11,'Material',0,7,1),(12,'Female',1,8,2),(13,'Male',1,8,2),(14,'Fixture',0,9,1),(15,'Instrumentality',0,9,3),(16,'Opening',0,9,10),(17,'Padding',0,9,1),(18,'Structure',0,9,1),(19,'Decoration',0,9,1),(20,'Commodity',0,9,1),(21,'Article',0,9,2),(22,'Way',0,9,6),(23,'Organism',0,10,1),(24,'Paper',0,11,1),(25,'PlumbingFixture',0,14,1),(26,'Furnishing',0,15,2),(27,'Device',0,15,1),(28,'Container',0,15,1),(29,'Implement',0,15,1),(30,'Equipment',0,15,1),(31,'Window',0,16,7),(32,'Cushion',0,17,3),(33,'Area',0,18,5),(34,'Obstruction',0,18,1),(35,'Design',0,19,4),(36,'ConsumerGoods',0,20,1),(37,'Ware',0,21,1),(38,'Passage',0,22,3),(39,'Plant',0,23,2),(40,'Animal',0,23,1),(41,'Tissue',0,24,2),(42,'Toilet',0,25,2),(43,'Sink',0,25,1),(44,'Furniture',0,26,1),(45,'SourceOfIllumination',0,27,1),(46,'Machine',0,27,1),(47,'Support',0,27,10),(48,'Restraint',0,27,6),(49,'ElectricalDevice',0,27,1),(50,'WheeledVehicle',0,28,1),(51,'Cup',0,28,1),(52,'Spoon',0,28,1),(53,'Glass',0,28,2),(54,'Receptacle',0,28,1),(55,'Can',0,28,1),(56,'Bin',0,28,1),(57,'Vessel',0,28,3),(58,'Box',0,28,1),(59,'Utensil',0,29,1),(60,'Tool',0,29,1),(61,'ElectronicEquipment',0,30,1),(62,'Pillow',0,32,1),(63,'Room',0,33,1),(64,'Barrier',0,34,1),(65,'Emblem',0,35,1),(66,'Durables',0,36,1),(67,'Tableware',0,37,1),(68,'Conduit',0,38,1),(69,'VascularPlant',0,39,1),(70,'Chordate',0,40,1),(71,'BathroomSink',0,43,-1),(72,'KitchenSink',0,43,-1),(73,'Table',0,44,2),(74,'Seat',0,44,3),(75,'Lamp',0,45,1),(76,'Computer',0,46,1),(77,'Printer',0,46,3),(78,'Shelf',0,47,1),(79,'Rack',0,47,5),(80,'Fastener',0,48,2),(81,'Battery',0,49,2),(82,'Bicycle',0,50,1),(83,'HurricaneGlass',0,53,-1),(84,'Tray',0,54,1),(85,'TrashCan',0,55,-1),(86,'RecycleBin',0,56,-1),(87,'DrinkingVessel',0,57,1),(88,'KitchenUtensil',0,59,1),(89,'CuttingImplement',0,60,1),(90,'Set',0,61,13),(91,'Bathroom',0,63,1),(92,'MovableBarrier',0,64,1),(93,'Flag',0,65,1),(94,'Appliance',0,66,2),(95,'Flatware',0,67,1),(96,'Cutlery',0,67,2),(97,'Tube',0,68,1),(98,'Vine',0,69,1),(99,'Vertebrate',0,70,1),(100,'Desk',0,73,1),(101,'Sofa',0,74,1),(102,'Chair',0,74,1),(103,'Candle',0,75,1),(104,'ClothesRack',0,79,-1),(105,'Lock',0,80,1),(106,'TeaTray',0,84,-1),(107,'Mug',0,87,4),(108,'CookingUtensil',0,88,1),(109,'Cutter',0,89,6),(110,'Receiver',0,90,1),(111,'Door',0,92,1),(112,'Dryer',0,94,1),(113,'HomeAppliance',0,94,1),(114,'Plate',0,95,4),(115,'Fork',0,96,1),(116,'Hose',0,97,3),(117,'Mammal',0,99,1),(118,'LoveSeat',0,101,1),(119,'Pan',0,108,1),(120,'Pot',0,108,1),(121,'EdgeTool',0,109,1),(122,'TelevisionReceiver',0,110,1),(123,'KitchenAppliance',0,113,1),(124,'Placental',0,117,1),(125,'Loveseat',0,118,-1),(126,'Knife',0,121,1),(127,'Television',0,122,-1),(128,'Oven',0,123,1),(129,'Stove',0,123,1),(130,'Carnivore',0,124,1),(131,'Feline',0,130,1),(132,'Cat',0,131,1);
/*!40000 ALTER TABLE `object` ENABLE KEYS */;
UNLOCK TABLES;


DROP TABLE IF EXISTS `property_type`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `property_type` (
  `prop_id` int(10) unsigned NOT NULL,
  `prop_name` varchar(45) DEFAULT NULL,
  `is_int` tinyint(1) DEFAULT '1',
  `omega` int(11) DEFAULT '0' COMMENT 'Omega determines if the property is an action property, and object property, or can work for both. Values in PAR''s system are:\n0 is object (default)\n1 is action\n2 is both',
  PRIMARY KEY (`prop_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `property_type`
--

LOCK TABLES `property_type` WRITE;
/*!40000 ALTER TABLE `property_type` DISABLE KEYS */;
INSERT INTO `property_type` VALUES (1,'obj_status',0,0),(5,'obj_hue',1,0),(6,'obj_saturation',1,0),(7,'obj_brightness',1,0),(8,'obj_luminance',1,0),(9,'obj_opacity',1,0),(10,'obj_reflectivity',1,0),(11,'obj_refraction',1,0),(12,'obj_sound_decible',1,0),(13,'obj_sound_freq',1,0),(14,'obj_smell_intensity',1,0),(15,'obj_temperature',1,0),(16,'manner',0,1);
/*!40000 ALTER TABLE `property_type` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `site`
--

DROP TABLE IF EXISTS `site`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `site` (
  `obj_id` int(10) unsigned NOT NULL,
  `site_type_id` int(10) unsigned NOT NULL,
  `site_pos_x` float DEFAULT NULL,
  `site_pos_y` float DEFAULT NULL,
  `site_pos_z` float DEFAULT NULL,
  `site_orient_x` float DEFAULT NULL,
  `site_orient_y` float DEFAULT NULL,
  `site_orient_z` float DEFAULT NULL,
  `site_shape_id` int(11) DEFAULT '-1',
  PRIMARY KEY (`obj_id`,`site_type_id`),
  KEY `site_type` (`site_type_id`),
  CONSTRAINT `object_check` FOREIGN KEY (`obj_id`) REFERENCES `object` (`obj_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `site_type` FOREIGN KEY (`site_type_id`) REFERENCES `site_type` (`site_type_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `site`
--

LOCK TABLES `site` WRITE;
/*!40000 ALTER TABLE `site` DISABLE KEYS */;
INSERT INTO `site` VALUES (1,0,0,0,0,0,0,0,-1),(1,1,0,0,0,0,0,0,-1),(1,2,0,0,0,0,0,0,1);
/*!40000 ALTER TABLE `site` ENABLE KEYS */;
UNLOCK TABLES;


--
-- Table structure for table `site_shape`
--

DROP TABLE IF EXISTS `site_shape`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `site_shape` (
  `site_shape_id` int(11) NOT NULL,
  `shape_type` varchar(45) DEFAULT 'box',
  `first_coord` float DEFAULT '0.1',
  `second_coord` float DEFAULT '0.1',
  `third_coord` float DEFAULT '0.1',
  PRIMARY KEY (`site_shape_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `site_shape`
--

LOCK TABLES `site_shape` WRITE;
/*!40000 ALTER TABLE `site_shape` DISABLE KEYS */;
INSERT INTO `site_shape` VALUES (1,'box',1,1,1),(2,'capsule',30,50,50);
/*!40000 ALTER TABLE `site_shape` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `site_type`
--

DROP TABLE IF EXISTS `site_type`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `site_type` (
  `site_type_id` int(10) unsigned NOT NULL,
  `site_name` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`site_type_id`),
  UNIQUE KEY `site_name_UNIQUE` (`site_name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `site_type`
--

LOCK TABLES `site_type` WRITE;
/*!40000 ALTER TABLE `site_type` DISABLE KEYS */;
INSERT INTO `site_type` VALUES (3,'grasp'),(2,'inspect'),(0,'operate'),(1,'orient_only');
/*!40000 ALTER TABLE `site_type` ENABLE KEYS */;
UNLOCK TABLES;

--

