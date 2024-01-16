pub struct Hole {
    pub g_v:    (String,u16,String), //630 표준편차 12 mm
    pub g_u:    (String,u8,String),  //70
    pub g_a:    (String,u16,String), //
    pub p_v:    (String,u16,String), //mmhg(torr) 410 to 980
    pub p_u:    (String,u16,String), //260, //190 = 요실금
    pub p_a:    (String,u16,String), //mmhg(torr) 456 to 1388
}
impl Hole{
    pub fn new(id:String,species:String) -> Hole {
        Hole {
            g_v: (id.clone(),dice::gaussian_rand(555,40).unwrap(),     species.clone()),
            g_u: (id.clone(),dice::gaussian_rand(65,4).unwrap() as u8, species.clone()),
            g_a: (id.clone(),dice::gaussian_rand(418,21).unwrap(),     species.clone()),
            p_v: (id.clone(),dice::gaussian_rand(695,80).unwrap(),     species.clone()),
            p_u: (id.clone(),dice::gaussian_rand(250,30).unwrap(),     species.clone()),
            p_a: (id.clone(),dice::gaussian_rand(922,130).unwrap(),    species.clone()),
        }
    }
    pub fn meiosis(id:String,mother:Hole,father:Hole) -> Hole {
        let true_false  = dice::rand_array_tf(5,1);
        let mutation    = dice::rand_array_tf(5,100);
        Hole {
            g_v: mutation_u16_s(id.clone(), mother.g_v, father.g_v, mutation[0], true_false[0]),
            g_u: mutation_u8_s( id.clone(), mother.g_u, father.g_u, mutation[1], true_false[1]),
            g_a: mutation_u16_s(id.clone(), mother.g_a, father.g_a, mutation[2], true_false[2]),
            p_v: mutation_u16_s(id.clone(), mother.p_v, father.p_v, mutation[3], true_false[3]),
            p_u: mutation_u16_s(id.clone(), mother.p_u, father.p_u, mutation[3], true_false[3]),
            p_a: mutation_u16_s(id.clone(), mother.p_a, father.p_a, mutation[4], true_false[4]),
        }
    }
    pub fn blend(mother:Hole,father:Hole) -> (Vec<String>,String) {
        let g_v = heredity_u16_s(mother.g_v.1, father.g_v.1);
        let g_u = heredity_u8_s(mother.g_u.1, father.g_u.1);
        let g_a = heredity_u16_s(mother.g_a.1, father.g_a.1);
        let p_v = heredity_u16_s(mother.p_v.1, father.p_v.1);
        let p_u = heredity_u16_s(mother.p_u.1, father.p_u.1);
        let p_a = heredity_u16_s(mother.p_a.1, father.p_a.1);
        let response:String = "{\r\n \"g_v\":".to_owned() + &g_v.to_string() + ",\r\n"+
                                   " \"g_u\":"            + &g_u.to_string() + ",\r\n"+
                                   " \"g_a\":"            + &g_a.to_string() + ",\r\n"+
                                   " \"p_v\":"            + &p_v.to_string() + ",\r\n"+
                                   " \"p_u\":"            + &p_u.to_string() + ",\r\n"+
                                   " \"p_a\":"            + &p_a.to_string() + "\r\n}";
        (vec![mother.g_v.2,father.g_v.2,
         mother.g_u.2,father.g_u.2,
         mother.g_a.2,father.g_a.2,
         mother.p_v.2,father.p_v.2,
         mother.p_u.2,father.p_u.2,
         mother.p_a.2,father.p_a.2]
        ,response)
    }
    pub fn save(&self) -> String {
        let response:String = 
            "{\r\n \"g_v\":[\"".to_owned() + &self.g_v.0 + "\","+ &self.g_v.1.to_string() + ",\""+ &self.g_v.2 + "\"],\r\n"+
                 " \"g_u\":[\""            + &self.g_u.0 + "\","+ &self.g_u.1.to_string() + ",\""+ &self.g_u.2 + "\"],\r\n"+
                 " \"g_a\":[\""            + &self.g_a.0 + "\","+ &self.g_a.1.to_string() + ",\""+ &self.g_a.2 + "\"],\r\n"+
                 " \"p_v\":[\""            + &self.p_v.0 + "\","+ &self.p_v.1.to_string() + ",\""+ &self.p_v.2 + "\"],\r\n"+
                 " \"p_u\":[\""            + &self.p_u.0 + "\","+ &self.p_u.1.to_string() + ",\""+ &self.p_u.2 + "\"],\r\n"+
                 " \"p_a\":[\""            + &self.p_a.0 + "\","+ &self.p_a.1.to_string() + ",\""+ &self.p_a.2 + "\"]\r\n}";
        response
    }
    pub fn load(data:String) -> Result<Hole> {
        let model: Value = serde_json::from_str(data.as_str())?;        
        let response = Hole {    
            g_v: (json::string(&model["g_v"][0]), json::u16(&model["g_v"][1]), json::string(&model["g_v"][2])),
            g_u: (json::string(&model["g_u"][0]), json::u8(&model["g_u"][1]),  json::string(&model["g_u"][2])),
            g_a: (json::string(&model["g_a"][0]), json::u16(&model["g_a"][1]), json::string(&model["g_a"][2])),
            p_v: (json::string(&model["p_v"][0]), json::u16(&model["p_v"][1]), json::string(&model["p_v"][2])),
            p_u: (json::string(&model["p_u"][0]), json::u16(&model["p_u"][1]), json::string(&model["p_u"][2])),
            p_a: (json::string(&model["p_a"][0]), json::u16(&model["p_a"][1]), json::string(&model["p_a"][2])),
        };
        Ok(response)
    }
}

pub struct Status {
    pub intelligence:   (String,u8,String),
    pub strength:       (String,u8,String),
    pub dexterity:      (String,u8,String),
    pub charisma:       (String,u8,String),     //MP
    pub constitution:   (String,u8,String),     //HP
}
impl Status{
    pub fn new(id:String,species:String) -> Status {
        let rand_8      = dice::rand_array_8(5,100);
        Status {    
            intelligence: (id.clone(),rand_8[0], species.clone()),
            strength:     (id.clone(),rand_8[1], species.clone()),
            dexterity:    (id.clone(),rand_8[2], species.clone()),
            charisma:     (id.clone(),rand_8[3], species.clone()),
            constitution: (id.clone(),rand_8[4], species.clone()),
        }
    }
    pub fn meiosis(id:String,mother:Status,father:Status) -> Status {
        let true_false  = dice::rand_array_tf(5,1);
        let mutation    = dice::rand_array_tf(5,100);
        Status {
            intelligence: mutation_u8_s(id.clone(), mother.intelligence, father.intelligence, mutation[0], true_false[0]),
            strength:     mutation_u8_s(id.clone(), mother.strength,     father.strength,     mutation[1], true_false[1]),
            dexterity:    mutation_u8_s(id.clone(), mother.dexterity,    father.dexterity,    mutation[2], true_false[2]),
            charisma:     mutation_u8_s(id.clone(), mother.charisma,     father.charisma,     mutation[3], true_false[3]), //마력
            constitution: mutation_u8_s(id.clone(), mother.constitution, father.constitution, mutation[4], true_false[4]), //체력
        }
    }
    pub fn blend(mother:Status,father:Status) -> (Vec<String>,String) {
        let intelligence = heredity_u8_s(mother.intelligence.1, father.intelligence.1);
        let strength     = heredity_u8_s(mother.strength.1,     father.strength.1);
        let dexterity    = heredity_u8_s(mother.dexterity.1,    father.dexterity.1);
        let charisma     = heredity_u8_s(mother.charisma.1,     father.charisma.1);
        let constitution = heredity_u8_s(mother.constitution.1, father.constitution.1);
        let response:String = "{\r\n \"intelligence\":".to_owned() + &intelligence.to_string() + ",\r\n"+
                                   " \"strength\":"                + &strength.to_string()     + ",\r\n"+
                                   " \"dexterity\":"               + &dexterity.to_string()    + ",\r\n"+
                                   " \"charisma\":"                + &charisma.to_string()     + ",\r\n"+
                                   " \"constitution\":"            + &constitution.to_string() + "\r\n}";
        (vec![mother.intelligence.2,father.intelligence.2,
         mother.strength.2,father.strength.2,
         mother.dexterity.2,father.dexterity.2,
         mother.charisma.2,father.charisma.2,
         mother.constitution.2,father.constitution.2]
        ,response)
    }
    pub fn save(&self) -> String {
        let response:String = 
            "{\r\n \"intelligence\":[\"".to_owned() + &self.intelligence.0 + "\","+ &self.intelligence.1.to_string() + ",\""+ &self.intelligence.2 + "\"],\r\n"+
                 " \"strength\":[\""                + &self.strength.0     + "\","+ &self.strength.1.to_string()     + ",\""+ &self.strength.2     + "\"],\r\n"+
                 " \"dexterity\":[\""               + &self.dexterity.0    + "\","+ &self.dexterity.1.to_string()    + ",\""+ &self.dexterity.2    + "\"],\r\n"+
                 " \"charisma\":[\""                + &self.charisma.0     + "\","+ &self.charisma.1.to_string()     + ",\""+ &self.charisma.2     + "\"],\r\n"+
                 " \"constitution\":[\""            + &self.constitution.0 + "\","+ &self.constitution.1.to_string() + ",\""+ &self.constitution.2 + "\"]\r\n}";
        response
    }
    pub fn load(data:String) -> Result<Status> {
        let model: Value = serde_json::from_str(data.as_str())?;
        let response = Status {    
            intelligence: (json::string(&model["intelligence"][0]), json::u8(&model["intelligence"][1]), json::string(&model["intelligence"][2])),
            strength:     (json::string(&model["strength"][0]),     json::u8(&model["strength"][1]),     json::string(&model["strength"][2])),
            dexterity:    (json::string(&model["dexterity"][0]),    json::u8(&model["dexterity"][1]),    json::string(&model["dexterity"][2])),
            charisma:     (json::string(&model["charisma"][0]),     json::u8(&model["charisma"][1]),     json::string(&model["charisma"][2])),
            constitution: (json::string(&model["constitution"][0]), json::u8(&model["constitution"][1]), json::string(&model["constitution"][2])),
        };
        Ok(response)
    }
}

pub struct Sense {
    pub cervix:       (String,u8,String),
    pub skin:         (String,u8,String),
    pub clit_glans:   (String,u8,String),
    pub vagina_balls: (String,u8,String),
    pub urethra:      (String,u8,String),
    pub anal:         (String,u8,String),
    pub nipple:       (String,u8,String),
}
impl Sense{
    pub fn new(id:String,species:String) -> Sense {
        let rand_8      = dice::rand_array_8(7,200);
        Sense {    
            cervix:       (id.clone(),rand_8[0], species.clone()),
            skin:         (id.clone(),rand_8[1], species.clone()),
            clit_glans:   (id.clone(),rand_8[2], species.clone()),
            vagina_balls: (id.clone(),rand_8[3], species.clone()),
            urethra:      (id.clone(),rand_8[4], species.clone()),
            anal:         (id.clone(),rand_8[5], species.clone()),
            nipple:       (id.clone(),rand_8[6], species.clone()),
        }
    }
    pub fn meiosis(id:String,mother:Sense,father:Sense) -> Sense {
        let true_false  = dice::rand_array_tf(7,1);
        let mutation    = dice::rand_array_tf(7,100);
        Sense {
            cervix:        mutation_u8_s(id.clone(), mother.cervix,   father.cervix,   mutation[0], true_false[0]),
            skin:          mutation_u8_s(id.clone(), mother.skin,   father.skin,   mutation[0], true_false[0]),
            clit_glans:    mutation_u8_s(id.clone(), mother.clit_glans,   father.clit_glans,   mutation[0], true_false[0]),
            vagina_balls:  mutation_u8_s(id.clone(), mother.vagina_balls, father.vagina_balls, mutation[1], true_false[1]),
            urethra:       mutation_u8_s(id.clone(), mother.urethra,      father.urethra,      mutation[2], true_false[2]),
            anal:          mutation_u8_s(id.clone(), mother.anal,         father.anal,         mutation[3], true_false[3]), //마력
            nipple:        mutation_u8_s(id.clone(), mother.nipple,       father.nipple,       mutation[4], true_false[4]), //체력
        }
    }
    pub fn blend(mother:Sense,father:Sense) -> (Vec<String>,String) {
        let cervix       = heredity_u8_s(mother.cervix.1,       father.cervix.1);
        let skin         = heredity_u8_s(mother.skin.1,         father.skin.1);
        let clit_glans   = heredity_u8_s(mother.clit_glans.1,   father.clit_glans.1);
        let vagina_balls = heredity_u8_s(mother.vagina_balls.1, father.vagina_balls.1);
        let urethra = heredity_u8_s(mother.urethra.1, father.urethra.1);
        let anal    = heredity_u8_s(mother.anal.1,    father.anal.1);
        let nipple  = heredity_u8_s(mother.nipple.1,  father.nipple.1);
        let response:String = "{\r\n \"cervix\":".to_owned() + &cervix.to_string()   + ",\r\n"+
                                   " \"skin\":"              + &skin.to_string() + ",\r\n"+
                                   " \"clit_glans\":"        + &clit_glans.to_string()   + ",\r\n"+
                                   " \"vagina_balls\":"      + &vagina_balls.to_string() + ",\r\n"+
                                   " \"urethra\":"           + &urethra.to_string()      + ",\r\n"+
                                   " \"anal\":"              + &anal.to_string()         + ",\r\n"+
                                   " \"nipple\":"            + &nipple.to_string()       + "\r\n}";
        (vec![mother.cervix.2,father.cervix.2,
        mother.skin.2,father.skin.2,
        mother.clit_glans.2,father.clit_glans.2,
         mother.vagina_balls.2,father.vagina_balls.2,
         mother.urethra.2,father.urethra.2,
         mother.anal.2,father.anal.2,
         mother.nipple.2,father.nipple.2]
        ,response)
    }
    pub fn save(&self) -> String {
        let response:String = 
            "{\r\n \"cervix\":[\"".to_owned() + &self.cervix.0       + "\"," + &self.cervix.1.to_string()       + ",\""+ &self.cervix.2       + "\"],\r\n"+
                 " \"skin\":[\""              + &self.skin.0         + "\"," + &self.skin.1.to_string()         + ",\""+ &self.skin.2         + "\"],\r\n"+
                 " \"clit_glans\":[\""        + &self.clit_glans.0   + "\"," + &self.clit_glans.1.to_string()   + ",\""+ &self.clit_glans.2   + "\"],\r\n"+
                 " \"vagina_balls\":[\""      + &self.vagina_balls.0 + "\"," + &self.vagina_balls.1.to_string() + ",\""+ &self.vagina_balls.2 + "\"],\r\n"+
                 " \"urethra\":[\""           + &self.urethra.0      + "\"," + &self.urethra.1.to_string()      + ",\""+ &self.urethra.2      + "\"],\r\n"+
                 " \"anal\":[\""              + &self.anal.0         + "\"," + &self.anal.1.to_string()         + ",\""+ &self.anal.2         + "\"],\r\n"+
                 " \"nipple\":[\""            + &self.nipple.0       + "\"," + &self.nipple.1.to_string()       + ",\""+ &self.nipple.2       + "\"]\r\n}";
        response
    }
    pub fn load(data:String) -> Result<Sense> {
        let model: Value = serde_json::from_str(data.as_str())?;
        let response = Sense {    
            cervix:       (json::string(&model["cervix"][0]),       json::u8(&model["cervix"][1]),      json::string(&model["cervix"][2])),
            skin:         (json::string(&model["skin"][0]),         json::u8(&model["skin"][1]),        json::string(&model["skin"][2])),
            clit_glans:   (json::string(&model["clit_glans"][0]),   json::u8(&model["clit_glans"][1]),   json::string(&model["clit_glans"][2])),
            vagina_balls: (json::string(&model["vagina_balls"][0]), json::u8(&model["vagina_balls"][1]), json::string(&model["vagina_balls"][2])),
            urethra:      (json::string(&model["urethra"][0]),      json::u8(&model["urethra"][1]),      json::string(&model["urethra"][2])),
            anal:         (json::string(&model["anal"][0]),         json::u8(&model["anal"][1]),         json::string(&model["anal"][2])),
            nipple:       (json::string(&model["nipple"][0]),       json::u8(&model["nipple"][1]),       json::string(&model["nipple"][2])),
        };
        Ok(response)
    }
}

pub struct Nature {   
    pub at_e_i: (String,u8,String),
    pub at_s_n: (String,u8,String),
    pub fn_t_f: (String,u8,String),
    pub fn_j_p: (String,u8,String),
}
impl Nature{
    pub fn new(id:String,species:String) -> Nature {
        let rand_8  = dice::rand_array_8(4,200);
        Nature {    
            at_e_i: (id.clone(),rand_8[0], species.clone()),
            at_s_n: (id.clone(),rand_8[1], species.clone()),
            fn_t_f: (id.clone(),rand_8[2], species.clone()),
            fn_j_p: (id.clone(),rand_8[3], species.clone()),
        }
    }
    pub fn meiosis(id:String,mother:Nature,father:Nature) -> Nature {
        let true_false  = dice::rand_array_tf(4,1);
        let mutation    = dice::rand_array_tf(4,100);
        Nature {
            at_e_i: mutation_u8_s(id.clone(), mother.at_e_i, father.at_e_i, mutation[0], true_false[0]),
            at_s_n: mutation_u8_s(id.clone(), mother.at_s_n, father.at_s_n, mutation[1], true_false[1]),
            fn_t_f: mutation_u8_s(id.clone(), mother.fn_t_f, father.fn_t_f, mutation[2], true_false[2]),
            fn_j_p: mutation_u8_s(id.clone(), mother.fn_j_p, father.fn_j_p, mutation[3], true_false[3]),
        }
    }
    pub fn blend(mother:Nature,father:Nature) -> (Vec<String>,String) {
        let at_e_i = heredity_u8_s(mother.at_e_i.1, father.at_e_i.1);
        let at_s_n = heredity_u8_s(mother.at_s_n.1, father.at_s_n.1);
        let fn_t_f = heredity_u8_s(mother.fn_t_f.1, father.fn_t_f.1);
        let fn_j_p = heredity_u8_s(mother.fn_j_p.1, father.fn_j_p.1);
        let mut mbti:String = if at_e_i > 50 {"E".to_owned()} else {"I".to_owned()};
        mbti += if at_s_n > 50 {"S"} else {"N"};
        mbti += if fn_t_f > 50 {"T"} else {"F"};
        mbti += if fn_j_p > 50 {"J"} else {"P"};
        let response:String = "{\r\n \"mbti\":\"".to_owned()   + &mbti + "\",\r\n"+
                                   " \"at_e_i\":"            + &at_e_i.to_string() + ",\r\n"+
                                   " \"at_s_n\":"            + &at_s_n.to_string() + ",\r\n"+
                                   " \"fn_t_f\":"            + &fn_t_f.to_string() + ",\r\n"+
                                   " \"fn_j_p\":"            + &fn_j_p.to_string() + "\r\n}";
        (vec![mother.at_e_i.2,father.at_e_i.2,
         mother.at_s_n.2,father.at_s_n.2,
         mother.fn_t_f.2,father.fn_t_f.2,
         mother.fn_j_p.2,father.fn_j_p.2]
        ,response)
    }
    pub fn save(&self) -> String {
        let response:String = 
            "{\r\n \"at_e_i\":[\"".to_owned() + &self.at_e_i.0 + "\","+ &self.at_e_i.1.to_string() + ",\""+ &self.at_e_i.2 + "\"],\r\n"+
                 " \"at_s_n\":[\""            + &self.at_s_n.0 + "\","+ &self.at_s_n.1.to_string() + ",\""+ &self.at_s_n.2 + "\"],\r\n"+
                 " \"fn_t_f\":[\""            + &self.fn_t_f.0 + "\","+ &self.fn_t_f.1.to_string() + ",\""+ &self.fn_t_f.2 + "\"],\r\n"+
                 " \"fn_j_p\":[\""            + &self.fn_j_p.0 + "\","+ &self.fn_j_p.1.to_string() + ",\""+ &self.fn_j_p.2 + "\"]\r\n}";
        response
    }
    pub fn load(data:String) -> Result<Nature> {
        let model: Value = serde_json::from_str(data.as_str())?;
        let response = Nature {    
            at_e_i: (json::string(&model["at_e_i"][0]), json::u8(&model["at_e_i"][1]), json::string(&model["at_e_i"][2])),
            at_s_n: (json::string(&model["at_s_n"][0]), json::u8(&model["at_s_n"][1]), json::string(&model["at_s_n"][2])),
            fn_t_f: (json::string(&model["fn_t_f"][0]), json::u8(&model["fn_t_f"][1]), json::string(&model["fn_t_f"][2])),
            fn_j_p: (json::string(&model["fn_j_p"][0]), json::u8(&model["fn_j_p"][1]), json::string(&model["fn_j_p"][2])),
        };
        Ok(response)
    }
}

pub struct Eros {   
    pub lust:       (String,u8,String),
    pub sadism:     (String,u8,String),
    pub masohism:   (String,u8,String),
    pub exhibition: (String,u8,String),
    pub service:    (String,u8,String),
}
impl Eros{
    pub fn new(id:String,species:String) -> Eros {
        let rand_8      = dice::rand_array_8(5,200);
        Eros {    
            lust:       (id.clone(),rand_8[0], species.clone()),
            sadism:     (id.clone(),rand_8[1], species.clone()),
            masohism:   (id.clone(),rand_8[2], species.clone()),
            exhibition: (id.clone(),rand_8[3], species.clone()),
            service:    (id.clone(),rand_8[4], species.clone()),
        }
    }
    pub fn meiosis(id:String,mother:Eros,father:Eros) -> Eros {
        let true_false  = dice::rand_array_tf(5,1);
        let mutation    = dice::rand_array_tf(5,100);
        Eros {
            lust:       mutation_u8_s(id.clone(), mother.lust,       father.lust,       mutation[0], true_false[0]),
            sadism:     mutation_u8_s(id.clone(), mother.sadism,     father.sadism,     mutation[1], true_false[1]),
            masohism:   mutation_u8_s(id.clone(), mother.masohism,   father.masohism,   mutation[2], true_false[2]),
            exhibition: mutation_u8_s(id.clone(), mother.exhibition, father.exhibition, mutation[3], true_false[3]),
            service:    mutation_u8_s(id.clone(), mother.service,    father.service,    mutation[4], true_false[4]),
        }
    }
    pub fn blend(mother:Eros,father:Eros) -> (Vec<String>,String) {
        let lust        = heredity_u8_s(mother.lust.1,    father.lust.1);
        let sadism      = heredity_u8_s(mother.sadism.1,  father.sadism.1);
        let masohism    = heredity_u8_s(mother.masohism.1, father.masohism.1);
        let exhibition  = heredity_u8_s(mother.exhibition.1,    father.exhibition.1);
        let service     = heredity_u8_s(mother.service.1,  father.service.1);
        let response:String = "{\r\n \"lust\":".to_owned() + &lust.to_string()       + ",\r\n"+
                                   " \"sadism\":"          + &sadism.to_string()     + ",\r\n"+
                                   " \"masohism\":"        + &masohism.to_string()   + ",\r\n"+
                                   " \"exhibition\":"      + &exhibition.to_string() + ",\r\n"+
                                   " \"service\":"         + &service.to_string()    + "\r\n}";
        (vec![mother.lust.2,father.lust.2,
         mother.sadism.2,father.sadism.2,
         mother.masohism.2,father.masohism.2,
         mother.exhibition.2,father.exhibition.2,
         mother.service.2,father.service.2]
        ,response)
    }
    pub fn save(&self) -> String {
        let response:String = 
            "{\r\n \"lust\":[\"".to_owned() + &self.lust.0       + "\","+ &self.lust.1.to_string()       + ",\""+ &self.lust.2       + "\"],\r\n"+
                 " \"sadism\":[\""          + &self.sadism.0     + "\","+ &self.sadism.1.to_string()     + ",\""+ &self.sadism.2     + "\"],\r\n"+
                 " \"masohism\":[\""        + &self.masohism.0   + "\","+ &self.masohism.1.to_string()   + ",\""+ &self.masohism.2   + "\"],\r\n"+
                 " \"exhibition\":[\""      + &self.exhibition.0 + "\","+ &self.exhibition.1.to_string() + ",\""+ &self.exhibition.2 + "\"],\r\n"+
                 " \"service\":[\""         + &self.service.0    + "\","+ &self.service.1.to_string()    + ",\""+ &self.service.2    + "\"]\r\n}";
        response
    }
    pub fn load(data:String) -> Result<Eros> {
        let model: Value = serde_json::from_str(data.as_str())?;        
        let response = Eros {    
            lust:       (json::string(&model["lust"][0]),       json::u8(&model["lust"][1]),       json::string(&model["lust"][2])),
            sadism:     (json::string(&model["sadism"][0]),     json::u8(&model["sadism"][1]),     json::string(&model["sadism"][2])),
            masohism:   (json::string(&model["masohism"][0]),   json::u8(&model["masohism"][1]),   json::string(&model["masohism"][2])),
            exhibition: (json::string(&model["exhibition"][0]), json::u8(&model["exhibition"][1]), json::string(&model["exhibition"][2])),
            service:    (json::string(&model["service"][0]),    json::u8(&model["service"][1]),    json::string(&model["service"][2])),
        };
        Ok(response)
    }
}

////funtion
fn heredity_str(mother:String, father:String, dominant:bool) -> String {
    if dominant {
        mother
    } else {
        father
    }
}

fn heredity_bool(mother:bool, father:bool, dominant:bool) -> bool {
    if dominant {
        if mother || father {true} else {false}
    } else {
        if mother && father {true} else {false}
    }
}

fn heredity_u8_s(mother:u8, father:u8) -> u8 {
    let gene = sort_asc_u8(mother,father);
    dice::rand_u8(gene.0,gene.1)
}

fn heredity_u8_d(mother_f:u8,mother_m:u8,father_f:u8,father_m:u8,gender:bool,ratio_f:u16,ratio_m:u16) -> u8 {
    let mother  = if gender {((mother_f as u16 + (mother_m as u16 /10*ratio_m/10))/2) as u8} else {((((mother_f/10) as u16*ratio_f/10) + mother_m as u16)/2) as u8};
    let father  = if gender {((father_f as u16 + (father_m as u16 /10*ratio_m/10))/2) as u8} else {((((mother_f/10) as u16*ratio_f/10) + mother_m as u16)/2) as u8};
    let gene = sort_asc_u8(mother,father);
    dice::rand_u8(gene.0,gene.1)
}

fn heredity_u16_s(mother:u16, father:u16) -> u16 {
    let gene = sort_asc_u16(mother,father);
    dice::rand_u16(gene.0,gene.1)
}

fn heredity_u816(mother_f:u8,mother_m:u16,father_f:u8,father_m:u16,gender:bool,ratio_f:u16,ratio_m:u16) -> u16 {
    let mother:u16  = if gender {(mother_f as u16 + (mother_m/100*ratio_m))/2} else {((mother_f as u16 /10*ratio_f/10) + mother_m)/2};
    let father:u16  = if gender {(father_f as u16 + (father_m/100*ratio_m))/2} else {((mother_f as u16 /10*ratio_f/10) + mother_m)/2};
    let gene = sort_asc_u16(mother,father);
    dice::rand_u16(gene.0,gene.1)
}

fn heredity_u16_d(mother_f:u16,mother_m:u16,father_f:u16,father_m:u16,gender:bool,ratio_f:u16,ratio_m:u16) -> u16 {
    let mother  = if gender {(mother_f + (mother_m/100*ratio_m))/2} else {((mother_f/100*ratio_f) + mother_m)/2};
    let father  = if gender {(father_f + (father_m/100*ratio_m))/2} else {((mother_f/100*ratio_f) + mother_m)/2};
    let gene = sort_asc_u16(mother,father);
    dice::rand_u16(gene.0,gene.1)
}

fn mutation_string_s(mother:(String,String,String),father:(String,String,String),true_false:bool) -> (String,String,String){
    if true_false {mother}
    else {father}
}

fn mutation_bool_s(id:String,mother:(String,bool,String),father:(String,bool,String),mutation:bool,true_false:bool) -> (String,bool,String){
    if mutation {
        (id,dice::true_false(),if dice::true_false() {mother.2} else {father.2})
    }   
    else if true_false {mother}
    else {father}
}

fn mutation_u8_s(id:String,mother:(String,u8,String),father:(String,u8,String),mutation:bool,true_false:bool) -> (String,u8,String){
    if mutation {
        (id,if mother.1 > father.1 {dice::rand_u8(father.1,mother.1)} else {dice::rand_u8(mother.1,father.1)},if dice::true_false() {mother.2} else {father.2})
    }   
    else if true_false {mother}
    else {father}
}

fn mutation_u8_d(id:String,mother:(String,u8,u8,String),father:(String,u8,u8,String),mutation:bool,true_false:bool) -> (String,u8,u8,String){
    if mutation {
        (id,
            if mother.1 > father.1 {dice::rand_u8(father.1,mother.1)} else {dice::rand_u8(mother.1,father.1)},
            if mother.2 > father.2 {dice::rand_u8(father.2,mother.2)} else {dice::rand_u8(mother.2,father.2)},
        if dice::true_false() {mother.3} else {father.3})
    }   
    else if true_false {mother}
    else {father}
}

fn mutation_u816(id:String,mother:(String,u8,u16,String),father:(String,u8,u16,String),mutation:bool,true_false:bool) -> (String,u8,u16,String){
    if mutation {
        (id,
            if mother.1 > father.1 {dice::rand_u8(father.1,mother.1)} else {dice::rand_u8(mother.1,father.1)},
            if mother.2 > father.2 {dice::rand_u16(father.2,mother.2)} else {dice::rand_u16(mother.2,father.2)},
        if dice::true_false() {mother.3} else {father.3})
    }   
    else if true_false {mother}
    else {father}
}

fn mutation_u16_s(id:String,mother:(String,u16,String),father:(String,u16,String),mutation:bool,true_false:bool) -> (String,u16,String){
    if mutation {
        (id,if mother.1 > father.1 {dice::rand_u16(father.1,mother.1)} else {dice::rand_u16(mother.1,father.1)},if dice::true_false() {mother.2} else {father.2})
    }   
    else if true_false {mother}
    else {father}
}

fn mutation_u16_d(id:String,mother:(String,u16,u16,String),father:(String,u16,u16,String),mutation:bool,true_false:bool) -> (String,u16,u16,String){
    if mutation {
        (id,
            if mother.1 > father.1 {dice::rand_u16(father.1,mother.1)} else {dice::rand_u16(mother.1,father.1)},
            if mother.2 > father.2 {dice::rand_u16(father.2,mother.2)} else {dice::rand_u16(mother.2,father.2)},
        if dice::true_false() {mother.3} else {father.3})
    }   
    else if true_false {mother}
    else {father}
}

fn sort_asc_u8(data1:u8, data2:u8) -> (u8,u8){
    if data1 < data2 {
        (data1, data2)
    } else {
        (data2, data1)
    }
}

fn sort_asc_u16(data1:u16, data2:u16) -> (u16,u16){
    if data1 < data2 {
        (data1, data2)
    } else {
        (data2, data1)
    }
}