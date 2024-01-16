
pub struct Erogenous {    
    pub hood_c:         (String,String,String),     // 살짝 접힌, 완전히 접힌, 매끄럽게 이어진, 이중으로... //17%,38%,5%,18%,20%,2% //누적 17,55,60,78,98,100
    pub hood_start:     (String,u8,String),         // 둥근, 삼각, 일자, 역삼각, //15%,23%,55%,7%         //누적 15,38,93,100
    pub hood_texture:   (String,u8,String),         // 반들반들한, 매끄러운, 보드라운, 주름진 // 11%,32%,45%,12% //누적 11,43,88,100
    pub hood_width:     (String,u16,String),        // 7mm~23mm   //max33% = 124*0.33 = 41 //평균값 140, 임의의 표준편차 30
    pub hood_length:    (String,u16,String),        // 13mm~38mm  /평균값 260, 임의의 표준편차 40

    pub lip_i_shape:    (String,String,String),     // 일자,둥근,다이아몬드,도자기,하트,잠자리,병//22%,15%,10%,33%,8%,8%,4%  //누적 22,37,47,80,88,96,100
    pub lip_i_texture:  (String,u8,String),         // 아름다운, 매력적인, 음란한, 걸래같은// 7%,19%,49%,25%                //누적 7,26,75,100
    pub lip_i_width:    (String,u8,String),         // 2mm to 14mm   //평균 50, 임의의 표준편차 10
    pub lip_i_length:   (String,u16,String),        // 31mm~86mm    //평균 520, 임의의 표준편차 80
    pub lip_i_length_r: (String,u16,String),        // 3mm~36mm max 62  //평균 190, 임의의 표준편차 45
    pub lip_i_length_l: (String,u16,String),        // 3mm~36mm max 62  //평균 190, 임의의 표준편차 45
    pub lip_o_shape:    (String,u8,String),         // 엷은, 도톰한, 두툼한, 8%, 60%, 32%                   //누적 8,68,100
    pub lip_o_texture:  (String,u8,String),         // 아름다운, 매력적인, 음란한, 걸래같은// 7%,19%,49%,25% //누적 7,26,75,100
    
    pub clit_glans_d:   (String,u8,u16,String),     // clit:20 to 100, hood_width*0.3~0.4 :: // glans 평균값 260, 임의의 표준편차 20
    pub clit_glans_d_e: (String,u8,u16,String),     // clit:20 to 100, hood_width*0.5~0.6 :: // 평균값 340, 임의의 표준편차 40
    pub clit_glans_l:   (String,u8,u16,String),     // clit: hood_width*0.4~0.6 ::  // 평균값 270, 임의의 표준편차 30
    pub clit_glans_l_e: (String,u8,u16,String),     // clit: hood_width*0.6~0~9 ::  // 평균값 310, 임의의 표준편차 40
    
    pub body_d:         (String,u16,String),        // 평균값 29.6, 표준편차 2.9
    pub body_d_e:       (String,u16,String),        // 평균값 37.1, 표준편차 3.5
    pub body_l:         (String,u16,String),        // 평균값 916, 표준편차 15.7
    pub body_l_e:       (String,u16,String),        // 평균값 131.2, 표준편차 16.6    
    pub ball_r:         (String,u8,String),         //평균값 135, 표준편차 32 = over (ml/10)
    pub ball_l:         (String,u8,String),         //평균값 135, 표준편차 18 (ml/10)

    pub prepuce:        (String,bool,String),       // 열성(포경)
    pub invert:         (String,bool,String),       // 열성(함몰)    

    pub fork:           (String,u8,String),         // 매끄러운, 둥글게 주름진, 음란한// 8%,60%,32% //누적 8,68,100
    pub perineum:       (String,u8,String),         // 매끈한, 둥근, 주름진, 솔기 //66%,17%,10%,7%  //누적 66,83,93,100
    pub wrinkle:        (String,u8,String),         // 평균 36, 표준편차 6
    pub areola:         (String,u16,u16,String),    // 일본인 범위 20 to 70 // 한국인 표준편차 309 +-101 //터키 평균 36, //사우디 평균 45, 평균309. 임의의 표준편차 40  // 평균 260 표준편차 25
    pub nipple_d:       (String,u8,u8,String),      // divide 3.5 by size of areola, 평균112. 임의의 표준편차 13    // *0.26 by size of areola 평균 68,임의의표준편차 13
    pub nipple_d_e:     (String,u8,u8,String),      // + 20~50                   // 5~10
    pub nipple_h:       (String,u8,u8,String),      // 평균90. 임의의 표준편차 20  // 평균 20, 표준편차 4
    pub nipple_h_e:     (String,u8,u8,String),      // + 50~80                    // + 10~20
}
impl Erogenous{
    pub fn new(id:String,species:String) -> Erogenous {
        let true_false  = dice::rand_array_tf(2,1);
        let rand_8      = dice::rand_array_8(9,100);
        let width_hood  = dice::gaussian_rand(140,30).unwrap();
        let clit_range1 = (width_hood.clone()*3/10) as u8;
        let clit_range2 = (width_hood.clone()*4/10) as u8;
        let clit_range3 = (width_hood.clone()*5/10) as u8;
        let clit_range4 = (width_hood.clone()*6/10) as u8;
        let clit_range5 = (width_hood.clone()*9/10) as u8;
        let clit_d      = sort_asc_u8(dice::rand_u8(clit_range1,clit_range2.clone()),dice::rand_u8(clit_range3,clit_range4.clone()));
        let clit_l      = sort_asc_u8(dice::rand_u8(clit_range2,clit_range4.clone()),dice::rand_u8(clit_range4,clit_range5));
        let glans_d     = sort_asc_u16(dice::gaussian_rand(260,20).unwrap(),dice::gaussian_rand(340,40).unwrap());
        let glans_l     = sort_asc_u16(dice::gaussian_rand(270,30).unwrap(),dice::gaussian_rand(310,40).unwrap());
        let body_d      = sort_asc_u16(dice::gaussian_rand(296,29).unwrap(),dice::gaussian_rand(370,35).unwrap());
        let body_l      = sort_asc_u16(dice::gaussian_rand(916,157).unwrap(),dice::gaussian_rand(1312,157).unwrap());
        let nipple_d_w  = dice::gaussian_rand(112,13).unwrap() as u8;
        let nipple_d_m  = dice::gaussian_rand(68,13).unwrap() as u8;
        let nipple_h_w  = dice::gaussian_rand(90,20).unwrap() as u8;
        let nipple_h_m  = dice::gaussian_rand(20,4).unwrap() as u8;        
   
        Erogenous {   
            hood_c:         (id.clone(), if rand_8[0]<17 {"살짝 접혀".to_owned()} else if rand_8[0]<55 {"완전히 접혀".to_owned()} else if rand_8[0]<60 {"매끄럽게".to_owned()} else if rand_8[0]<78 {"두곂으로 살짝접혀".to_owned()} else if rand_8[0]<60 {"두곂으로 완전히 접혀".to_owned()} else {"두곂으로 매끈하게".to_owned()}, species.clone()),
            hood_start:     (id.clone(), rand_8[1], species.clone()),
            hood_texture:   (id.clone(), rand_8[3], species.clone()),
            hood_width:     (id.clone(), width_hood, species.clone()),
            hood_length:    (id.clone(), dice::gaussian_rand(260,40).unwrap(), species.clone()),
            lip_i_shape:    (id.clone(), if rand_8[3]<22 {"일자".to_owned()} else if rand_8[3]<37 {"둥근".to_owned()} else if rand_8[3]<47 {"다이아몬드".to_owned()} else if rand_8[3]<80 {"도자기".to_owned()} else if rand_8[3]<88 {"하트".to_owned()} else if rand_8[3]<96 {"잠자리".to_owned()} else {"호리병".to_owned()}, species.clone()),
            lip_i_texture:  (id.clone(), rand_8[4], species.clone()),
            lip_i_width:    (id.clone(), dice::gaussian_rand(50,10).unwrap() as u8, species.clone()),
            lip_i_length:   (id.clone(), dice::gaussian_rand(520,80).unwrap(), species.clone()),
            lip_i_length_r: (id.clone(), dice::gaussian_rand(190,45).unwrap(), species.clone()),
            lip_i_length_l: (id.clone(), dice::gaussian_rand(190,45).unwrap(), species.clone()),
            lip_o_shape:    (id.clone(), rand_8[5], species.clone()),
            lip_o_texture:  (id.clone(), rand_8[6], species.clone()),
            clit_glans_d:   (id.clone(), clit_d.0, glans_d.0, species.clone()),
            clit_glans_d_e: (id.clone(), clit_d.1, glans_d.1, species.clone()),
            clit_glans_l:   (id.clone(), clit_l.0, glans_l.0, species.clone()),
            clit_glans_l_e: (id.clone(), clit_l.1, glans_l.1, species.clone()),
            body_d:         (id.clone(), body_d.0, species.clone()),
            body_d_e:       (id.clone(), body_d.1, species.clone()),
            body_l:         (id.clone(), body_l.0, species.clone()),
            body_l_e:       (id.clone(), body_l.1, species.clone()),
            ball_r:         (id.clone(), dice::gaussian_rand(135,18).unwrap() as u8, species.clone()),
            ball_l:         (id.clone(), dice::gaussian_rand(135,18).unwrap() as u8, species.clone()),
            prepuce:        (id.clone(), true_false[0], species.clone()),
            invert:         (id.clone(), true_false[1], species.clone()),
            fork:           (id.clone(), rand_8[7], species.clone()),
            perineum:       (id.clone(), rand_8[8], species.clone()),
            wrinkle:        (id.clone(), dice::gaussian_rand(36,6).unwrap() as u8, species.clone()),
            areola:         (id.clone(), dice::gaussian_rand(309,40).unwrap(), dice::gaussian_rand(260,25).unwrap(), species.clone()),
            nipple_d:       (id.clone(), nipple_d_w.clone(), nipple_d_m.clone(), species.clone()),
            nipple_d_e:     (id.clone(), nipple_d_w + dice::rand_u8(20,50), nipple_d_m + dice::rand_u8(5,10), species.clone()),
            nipple_h:       (id.clone(), nipple_h_w.clone(), nipple_h_m.clone(), species.clone()),
            nipple_h_e:     (id.clone(), nipple_h_w + dice::rand_u8(20,50), nipple_h_m + dice::rand_u8(10,20), species.clone()),
        }    
    }
    pub fn meiosis(id:String,mother:Erogenous,father:Erogenous) -> Erogenous {
        let mutation    = dice::rand_array_tf(31,100);
        let true_false  = dice::rand_array_tf(35,1);
        Erogenous {
            hood_c:         mutation_string_s(mother.hood_c,father.hood_c,           true_false[2]),
            hood_start:     mutation_u8_s(id.clone(),mother.hood_start,father.hood_start,       mutation[0],true_false[3]),
            hood_texture:   mutation_u8_s(id.clone(),mother.hood_texture,father.hood_texture,   mutation[1],true_false[4]),
            hood_width:     mutation_u16_s(id.clone(),mother.hood_width,father.hood_width,      mutation[2],true_false[5]),
            hood_length:    mutation_u16_s(id.clone(),mother.hood_length,father.hood_length,    mutation[3],true_false[6]),
            lip_i_shape:    mutation_string_s(mother.lip_i_shape,father.lip_i_shape,             true_false[7]),
            lip_i_texture:  mutation_u8_s(id.clone(),mother.lip_i_texture,father.lip_i_texture, mutation[4],true_false[8]),
            lip_i_width:    mutation_u8_s(  id.clone(),mother.lip_i_width,father.lip_i_width,   mutation[5],true_false[9]),
            lip_i_length:   mutation_u16_s( id.clone(),mother.lip_i_length,father.lip_i_length, mutation[6],true_false[10]),
            lip_i_length_r: mutation_u16_s( id.clone(),mother.lip_i_length_r,father.lip_i_length_r,mutation[7],true_false[11]),
            lip_i_length_l: mutation_u16_s( id.clone(),mother.lip_i_length_l,father.lip_i_length_l,mutation[8],true_false[12]),
            lip_o_shape:    mutation_u8_s(id.clone(),mother.lip_o_shape,father.lip_o_shape,     mutation[9],true_false[13]),
            lip_o_texture:  mutation_u8_s(id.clone(),mother.lip_o_texture,father.lip_o_texture, mutation[10],true_false[14]),
            clit_glans_d:   mutation_u816(id.clone(),mother.clit_glans_d,father.clit_glans_d,   mutation[11],true_false[15]),
            clit_glans_d_e: mutation_u816(id.clone(),mother.clit_glans_d_e,father.clit_glans_d_e,mutation[12],true_false[16]),
            clit_glans_l:   mutation_u816(id.clone(),mother.clit_glans_l,father.clit_glans_l,   mutation[13],true_false[17]),
            clit_glans_l_e: mutation_u816(id.clone(),mother.clit_glans_l_e,father.clit_glans_l_e,mutation[14],true_false[18]),
            body_d:         mutation_u16_s( id.clone(),mother.body_d,father.body_d,             mutation[15],true_false[19]),
            body_d_e:       mutation_u16_s( id.clone(),mother.body_d_e,father.body_d_e,         mutation[16],true_false[20]),
            body_l:         mutation_u16_s( id.clone(),mother.body_l,father.body_l,             mutation[17],true_false[21]),
            body_l_e:       mutation_u16_s( id.clone(),mother.body_l_e,father.body_l_e,         mutation[18],true_false[22]),
            ball_r:         mutation_u8_s(  id.clone(),mother.ball_r,father.ball_r,             mutation[19],true_false[23]),
            ball_l:         mutation_u8_s(  id.clone(),mother.ball_l,father.ball_l,             mutation[20],true_false[24]),
            prepuce:        mutation_bool_s( id.clone(),mother.prepuce,father.prepuce,          mutation[21],true_false[25]),
            invert:         mutation_bool_s( id.clone(),mother.invert,father.invert,            mutation[22],true_false[26]),
            fork:           mutation_u8_s(id.clone(),mother.fork,father.fork,                   mutation[23],true_false[27]),
            perineum:       mutation_u8_s(id.clone(),mother.perineum,father.perineum,           mutation[24],true_false[28]),
            wrinkle:        mutation_u8_s(  id.clone(),mother.wrinkle,father.wrinkle,           mutation[25],true_false[29]),
            areola:         mutation_u16_d( id.clone(),mother.areola,father.areola,             mutation[26],true_false[30]),
            nipple_d:       mutation_u8_d(  id.clone(),mother.nipple_d,father.nipple_d,         mutation[27],true_false[31]),
            nipple_d_e:     mutation_u8_d(  id.clone(),mother.nipple_d_e,father.nipple_d_e,     mutation[28],true_false[32]),
            nipple_h:       mutation_u8_d(  id.clone(),mother.nipple_h,father.nipple_h,         mutation[29],true_false[33]),
            nipple_h_e:     mutation_u8_d(  id.clone(),mother.nipple_h_e,father.nipple_h_e,     mutation[30],true_false[34]),
        }
    }
    pub fn blend(mother:Erogenous,father:Erogenous,gender:&bool) -> (Vec<String>,String) {
        let true_false      = dice::rand_array_tf(2,1);
        //common
        let mut clit_glans_d    = heredity_u816(mother.clit_glans_d.1,  mother.clit_glans_d.2,  father.clit_glans_d.1,  father.clit_glans_d.2,  *gender,19,531);
        let mut clit_glans_d_e  = heredity_u816(mother.clit_glans_d_e.1,mother.clit_glans_d_e.2,father.clit_glans_d_e.1,father.clit_glans_d_e.2,  *gender,22,441);
        let mut clit_glans_l    = heredity_u816(mother.clit_glans_l.1,  mother.clit_glans_l.2,  father.clit_glans_l.1,  father.clit_glans_l.2,  *gender,26,386);
        let mut clit_glans_l_e  = heredity_u816(mother.clit_glans_l_e.1,mother.clit_glans_l_e.2,father.clit_glans_l_e.1,father.clit_glans_l_e.2,*gender,34,295);
        if clit_glans_d > clit_glans_d_e {
            let temporary = clit_glans_d;
            clit_glans_d = clit_glans_d_e;
            clit_glans_d_e  = temporary;
        }
        if clit_glans_l > clit_glans_l_e {
            let temporary = clit_glans_l;
            clit_glans_l = clit_glans_l_e;
            clit_glans_l_e  = temporary;
        }
        let perineum_u8         = heredity_u8_s(mother.perineum.1,      father.perineum.1);
        let perineum            = if perineum_u8<66 {"깔끔한".to_owned()} else if perineum_u8<83 {"끝이 둥근".to_owned()} else if perineum_u8<93 {"주름진".to_owned()} else {"솔기로 이어진".to_owned()};
        let wrinkle             = heredity_u8_s(mother.wrinkle.1,       father.wrinkle.1,);
        let areola              = heredity_u16_d(mother.areola.1,       mother.areola.2,    father.areola.1,     father.areola.2,    *gender,119,84);
        let mut nipple_d        = heredity_u8_d(mother.nipple_d.1,      mother.nipple_d.2,  father.nipple_d.1,   father.nipple_d.2,  *gender,164,60);
        let mut nipple_d_e      = heredity_u8_d(mother.nipple_d_e.1,    mother.nipple_d_e.2,father.nipple_d_e.1, father.nipple_d_e.2,*gender,196,51);
        let mut nipple_h        = heredity_u8_d(mother.nipple_h.1,      mother.nipple_h.2,  father.nipple_h.1,   father.nipple_h.2,  *gender,450,22);
        let mut nipple_h_e      = heredity_u8_d(mother.nipple_h_e.1,    mother.nipple_h_e.2,father.nipple_h_e.1, father.nipple_h_e.2,*gender,357,28);
        if nipple_d > nipple_d_e {
            let temporary = nipple_d;
            nipple_d = nipple_d_e;
            nipple_d_e  = temporary;
        }
        if nipple_h > nipple_h_e {
            let temporary = nipple_h;
            nipple_h = nipple_h_e;
            nipple_h_e  = temporary;
        }
        //female
        let hood_c          = heredity_str(mother.hood_c.1,        father.hood_c.1,        true_false[0]);        
        let hood_start_u8   = heredity_u8_s(mother.hood_start.1,    father.hood_start.1);
        let hood_texture_u8 = heredity_u8_s(mother.hood_texture.1,  father.hood_texture.1);
        let hood_start      = if hood_start_u8<15 {"동그란".to_owned()} else if hood_start_u8<38 {"뾰족한".to_owned()} else if hood_start_u8<93 {"일자로 뻗은".to_owned()} else {"역삼각의".to_owned()};
        let hood_texture    = if hood_texture_u8<11 {"반들거리는".to_owned()} else if hood_texture_u8<43 {"매끄러운".to_owned()} else if hood_texture_u8<88 {"보드라운".to_owned()} else {"주름진".to_owned()};
        let hood_length     = heredity_u16_s(mother.hood_length.1,  father.hood_length.1,);     
        let hood_width      = dice::rand_u16(clit_glans_d/10*25,clit_glans_d/10*34);
        let hood            =  if hood_width<131 {"숨어있는".to_owned()} else if hood_width<145 {"빼꼼나온".to_owned()} else if hood_width<180 {"튀어나온".to_owned()} else {"과시하는".to_owned()};
        let lip_i_shape     = heredity_str(mother.lip_i_shape.1,    father.lip_i_shape.1,   true_false[1]);
        let lip_i_texture_u8= heredity_u8_s(mother.lip_i_texture.1,  father.lip_i_texture.1);
        let lip_i_texture   = if lip_i_texture_u8<7 {"아름다운".to_owned()} else if lip_i_texture_u8<26 {"매력적인".to_owned()} else if lip_i_texture_u8<75 {"음란한".to_owned()} else {"걸래같은".to_owned()};
        let lip_i_width     = heredity_u8_s(mother.lip_i_width.1,   father.lip_i_width.1,);
        let lip_i_length    = heredity_u16_s(mother.lip_i_length.1, father.lip_i_length.1,);
        let lip_i_length_r  = heredity_u16_s(mother.lip_i_length_r.1, father.lip_i_length_r.1,);
        let lip_i_length_l  = heredity_u16_s(mother.lip_i_length_l.1, father.lip_i_length_l.1,);     
        let lip_o_shape_u8  = heredity_u8_s(mother.lip_o_shape.1,    father.lip_o_shape.1);
        let lip_o_texture_u8= heredity_u8_s(mother.lip_o_texture.1,  father.lip_o_texture.1);        
        let lip_o_shape     = if lip_o_shape_u8<8 {"엷은".to_owned()} else if lip_o_shape_u8<68 {"도톰한".to_owned()} else {"두툼한".to_owned()};
        let lip_o_texture   = if lip_o_texture_u8<7 {"아름다운".to_owned()} else if lip_o_texture_u8<26 {"매력적인".to_owned()} else if lip_o_texture_u8<75 {"음란한".to_owned()} else {"걸래같은".to_owned()};
        let lips            = sort_asc_u16(lip_i_length_r,lip_i_length_l).1;
        let lip             = if lips<162 {"가려진".to_owned()} else if lips<191 {"살짝 비치는".to_owned()} else if lips<240 {"비집고 나온".to_owned()} else if lips<268 {"달랑거리는".to_owned()} else {"펄럭거리는".to_owned()};
        let fork_u8         = heredity_u8_s(mother.fork.1,           father.fork.1);
        let fork            = if fork_u8<8 {"매끄러운".to_owned()} else if fork_u8<68 {"둥글게 주름진".to_owned()} else {"음란한".to_owned()};
        //male
        let mut body_d      = heredity_u16_s(mother.body_d.1,       father.body_d.1,);
        let mut body_d_e    = heredity_u16_s(mother.body_d_e.1,     father.body_d_e.1,);
        let mut body_l      = heredity_u16_s(mother.body_l.1,       father.body_l.1,);
        let mut body_l_e    = heredity_u16_s(mother.body_l_e.1,     father.body_l_e.1,);
        let ball_r          = heredity_u8_s(mother.ball_r.1,        father.ball_r.1,);
        let ball_l          = heredity_u8_s(mother.ball_l.1,        father.ball_l.1,);
        if body_d > body_d_e {
            let temporary = body_d;
            body_d = body_d_e;
            body_d_e  = temporary;
        }            
        if body_l > body_l_e {
            let temporary = body_l;
            body_l = body_l_e;
            body_l_e  = temporary;
        }
        let response:String;
        /*
        let response:String = "{\r\n \"common\":{\r\n".to_owned()  +
                                  "  \"clit_glans_d\":"            + &clit_glans_d.to_string()   + ",\r\n"+
                                  "  \"clit_glans_d_e\":"          + &clit_glans_d_e.to_string() + ",\r\n"+
                                  "  \"clit_glans_l\":"            + &clit_glans_l.to_string()   + ",\r\n"+
                                  "  \"clit_glans_l_e\":"          + &clit_glans_l_e.to_string() + ",\r\n"+
                                  "  \"perineum\":\""              + &perineum.to_string()       + "\",\r\n"+
                                  "  \"wrinkle\":"                 + &wrinkle.to_string()        + ",\r\n"+
                                  "  \"areola\":"                  + &areola.to_string()         + ",\r\n"+
                                  "  \"nipple_d\":"                + &nipple_d.to_string()       + ",\r\n"+
                                  "  \"nipple_d_e\":"              + &nipple_d_e.to_string()     + ",\r\n"+
                                  "  \"nipple_h\":"                + &nipple_h.to_string()       + ",\r\n"+
                                  "  \"nipple_h_e\":"              + &nipple_h_e.to_string()     + "\r\n },\r\n"+
                                   " \"female\":{\r\n"              +
                                  "  \"hood_c\":\""                + &hood_c.to_string()         + "\",\r\n"+
                                  "  \"hood_start\":\""            + &hood_start.to_string()     + "\",\r\n"+
                                  "  \"hood_texture\":\""          + &hood_texture.to_string()   + "\",\r\n"+
                                  "  \"hood_length\":"             + &hood_length.to_string()    + ",\r\n"+
                                  "  \"hood_width\":"              + &hood_width.to_string()     + ",\r\n"+
                                  "  \"hood\":\""                  + &hood.to_string()           + "\",\r\n"+
                                  "  \"lip_i_shape\":\""           + &lip_i_shape.to_string()    + "\",\r\n"+
                                  "  \"lip_i_texture\":\""         + &lip_i_texture.to_string()  + "\",\r\n"+
                                  "  \"lip_i_width\":"             + &lip_i_width.to_string()    + ",\r\n"+
                                  "  \"lip_i_length\":"            + &lip_i_length.to_string()   + ",\r\n"+
                                  "  \"lip_i_length_r\":"          + &lip_i_length_r.to_string() + ",\r\n"+
                                  "  \"lip_i_length_l\":"          + &lip_i_length_l.to_string() + ",\r\n"+
                                  "  \"lip_o_shape\":\""           + &lip_o_shape.to_string()    + "\",\r\n"+
                                  "  \"lip_o_texture\":\""         + &lip_o_texture.to_string()  + "\",\r\n"+
                                  "  \"lip\":\""                   + &lip.to_string()            + "\",\r\n"+
                                  "  \"fork\":\""                  + &fork.to_string()           + "\"\r\n },\r\n"+
                                   " \"male\":{\r\n"                +
                                  "  \"body_d\":"                  + &body_d.to_string()         + ",\r\n"+
                                  "  \"body_d_e\":"                + &body_d_e.to_string()       + ",\r\n"+
                                  "  \"body_l\":"                  + &body_l.to_string()         + ",\r\n"+
                                  "  \"body_l_e\":"                + &body_l_e.to_string()       + ",\r\n"+
                                  "  \"ball_r\":"                  + &ball_r.to_string()         + ",\r\n"+
                                  "  \"ball_l\":"                  + &ball_l.to_string()         + "\r\n }\r\n}";
        */
        if *gender {
            response = "{\r\n \"common\":{\r\n".to_owned()  +
                        "  \"perineum\":\""              + &perineum.to_string()       + "\",\r\n"+
                        "  \"wrinkle\":"                 + &wrinkle.to_string()        + ",\r\n"+
                        "  \"areola\":"                  + &areola.to_string()         + ",\r\n"+
                        "  \"nipple_d\":"                + &nipple_d.to_string()       + ",\r\n"+
                        "  \"nipple_d_e\":"              + &nipple_d_e.to_string()     + ",\r\n"+
                        "  \"nipple_h\":"                + &nipple_h.to_string()       + ",\r\n"+
                        "  \"nipple_h_e\":"              + &nipple_h_e.to_string()     + "\r\n },\r\n"+
                        " \"male\":{\r\n"                +
                        "  \"glans_d\":"                 + &clit_glans_d.to_string()   + ",\r\n"+
                        "  \"glans_d_e\":"               + &clit_glans_d_e.to_string() + ",\r\n"+
                        "  \"glans_l\":"                 + &clit_glans_l.to_string()   + ",\r\n"+
                        "  \"glans_l_e\":"               + &clit_glans_l_e.to_string() + ",\r\n"+                        
                        "  \"body_d\":"                  + &body_d.to_string()         + ",\r\n"+
                        "  \"body_d_e\":"                + &body_d_e.to_string()       + ",\r\n"+
                        "  \"body_l\":"                  + &body_l.to_string()         + ",\r\n"+
                        "  \"body_l_e\":"                + &body_l_e.to_string()       + ",\r\n"+
                        "  \"ball_r\":"                  + &ball_r.to_string()         + ",\r\n"+
                        "  \"ball_l\":"                  + &ball_l.to_string()         + "\r\n }\r\n}";
        } else {
            response = "{\r\n \"common\":{\r\n".to_owned()  +
            "  \"perineum\":\""              + &perineum.to_string()       + "\",\r\n"+
            "  \"wrinkle\":"                 + &wrinkle.to_string()        + ",\r\n"+
            "  \"areola\":"                  + &areola.to_string()         + ",\r\n"+
            "  \"nipple_d\":"                + &nipple_d.to_string()       + ",\r\n"+
            "  \"nipple_d_e\":"              + &nipple_d_e.to_string()     + ",\r\n"+
            "  \"nipple_h\":"                + &nipple_h.to_string()       + ",\r\n"+
            "  \"nipple_h_e\":"              + &nipple_h_e.to_string()     + "\r\n },\r\n"+
             " \"female\":{\r\n"             +             
            "  \"clit_d\":"                  + &clit_glans_d.to_string()   + ",\r\n"+
            "  \"clit_d_e\":"                + &clit_glans_d_e.to_string() + ",\r\n"+
            "  \"clit_l\":"                  + &clit_glans_l.to_string()   + ",\r\n"+
            "  \"clit_l_e\":"                + &clit_glans_l_e.to_string() + ",\r\n"+
            "  \"hood_c\":\""                + &hood_c.to_string()         + "\",\r\n"+
            "  \"hood_start\":\""            + &hood_start.to_string()     + "\",\r\n"+
            "  \"hood_texture\":\""          + &hood_texture.to_string()   + "\",\r\n"+
            "  \"hood_length\":"             + &hood_length.to_string()    + ",\r\n"+
            "  \"hood_width\":"              + &hood_width.to_string()     + ",\r\n"+
            "  \"hood\":\""                  + &hood.to_string()           + "\",\r\n"+
            "  \"lip_i_shape\":\""           + &lip_i_shape.to_string()    + "\",\r\n"+
            "  \"lip_i_texture\":\""         + &lip_i_texture.to_string()  + "\",\r\n"+
            "  \"lip_i_width\":"             + &lip_i_width.to_string()    + ",\r\n"+
            "  \"lip_i_length\":"            + &lip_i_length.to_string()   + ",\r\n"+
            "  \"lip_i_length_r\":"          + &lip_i_length_r.to_string() + ",\r\n"+
            "  \"lip_i_length_l\":"          + &lip_i_length_l.to_string() + ",\r\n"+
            "  \"lip_o_shape\":\""           + &lip_o_shape.to_string()    + "\",\r\n"+
            "  \"lip_o_texture\":\""         + &lip_o_texture.to_string()  + "\",\r\n"+
            "  \"lip\":\""                   + &lip.to_string()            + "\",\r\n"+
            "  \"fork\":\""                  + &fork.to_string()           + "\"\r\n }\r\n}";
        }
        (vec![mother.clit_glans_d.3, father.clit_glans_d.3,
            mother.clit_glans_d_e.3,father.clit_glans_d_e.3,
            mother.clit_glans_l.3,  father.clit_glans_l.3,
            mother.clit_glans_l_e.3,father.clit_glans_l_e.3,
            mother.fork.2,          father.fork.2,
            mother.perineum.2,      father.perineum.2,
            mother.wrinkle.2,       father.wrinkle.2,
            mother.areola.3,        father.areola.3,
            mother.nipple_d.3,      father.nipple_d.3,
            mother.nipple_d_e.3,    father.nipple_d_e.3,
            mother.nipple_h.3,      father.nipple_h.3,
            mother.nipple_h_e.3,    father.nipple_h_e.3,
            mother.hood_c.2,        father.hood_c.2,
            mother.hood_start.2,    father.hood_start.2,
            mother.hood_texture.2,  father.hood_texture.2,
            mother.hood_length.2,   father.hood_length.2,
            mother.hood_width.2,    father.hood_width.2,
            mother.lip_i_shape.2,   father.lip_i_shape.2,
            mother.lip_i_texture.2, father.lip_i_texture.2,
            mother.lip_i_width.2,   father.lip_i_width.2,
            mother.lip_i_length.2,  father.lip_i_length.2,
            mother.lip_i_length_r.2,father.lip_i_length_r.2,
            mother.lip_i_length_l.2,father.lip_i_length_l.2,
            mother.lip_o_shape.2,   father.lip_o_shape.2,
            mother.lip_o_texture.2, father.lip_o_texture.2,
            mother.body_d.2,        father.body_d.2,
            mother.body_d_e.2,      father.body_d_e.2,
            mother.body_l.2,        father.body_l.2,
            mother.body_l_e.2,      father.body_l_e.2,
            mother.ball_r.2,        father.ball_r.2,
            mother.ball_l.2,        father.ball_l.2],response)
    }
    pub fn save(&self) -> String {
        let response:String = 
            "{\r\n \"hood_c\":[\"".to_owned()+ &self.hood_c.0 +"\",\""       + &self.hood_c.1.to_string()         + "\",\""+ &self.hood_c.2 +"\"],\r\n"        +
                 " \"hood_start\":[\""      + &self.hood_start.0 +"\","      + &self.hood_start.1.to_string()     + ",\""+ &self.hood_start.2 +"\"],\r\n"    +
                 " \"hood_texture\":[\""    + &self.hood_texture.0 +"\","    + &self.hood_texture.1.to_string()   + ",\""+ &self.hood_texture.2 +"\"],\r\n"  +
                 " \"hood_width\":[\""      + &self.hood_width.0 +"\","      + &self.hood_width.1.to_string()     + ",\""+ &self.hood_width.2 +"\"],\r\n"      +
                 " \"hood_length\":[\""     + &self.hood_length.0 +"\","     + &self.hood_length.1.to_string()    + ",\""+ &self.hood_length.2 +"\"],\r\n"     +
                 " \"lip_i_shape\":[\""     + &self.lip_i_shape.0 +"\",\""   + &self.lip_i_shape.1.to_string()    + "\",\""+ &self.lip_i_shape.2 +"\"],\r\n"   +
                 " \"lip_i_texture\":[\""   + &self.lip_i_texture.0 +"\","   + &self.lip_i_texture.1.to_string()  + ",\""+ &self.lip_i_texture.2 +"\"],\r\n" +
                 " \"lip_i_width\":[\""     + &self.lip_i_width.0 +"\","     + &self.lip_i_width.1.to_string()    + ",\""+ &self.lip_i_width.2 +"\"],\r\n"     +
                 " \"lip_i_length\":[\""    + &self.lip_i_length.0 +"\","    + &self.lip_i_length.1.to_string()   + ",\""+ &self.lip_i_length.2 +"\"],\r\n"    +
                 " \"lip_i_length_r\":[\""  + &self.lip_i_length_r.0 +"\","  + &self.lip_i_length_r.1.to_string() + ",\""+ &self.lip_i_length_r.2 +"\"],\r\n"  +
                 " \"lip_i_length_l\":[\""  + &self.lip_i_length_l.0 +"\","  + &self.lip_i_length_l.1.to_string() + ",\""+ &self.lip_i_length_l.2 +"\"],\r\n"  +
                 " \"lip_o_shape\":[\""     + &self.lip_o_shape.0 +"\","     + &self.lip_o_shape.1.to_string()    + ",\""+ &self.lip_o_shape.2 +"\"],\r\n"   +
                 " \"lip_o_texture\":[\""   + &self.lip_o_texture.0 +"\","   + &self.lip_o_texture.1.to_string()  + ",\""+ &self.lip_o_texture.2 +"\"],\r\n" +
                 " \"clit_glans_d\":[\""    + &self.clit_glans_d.0 +"\","    + &self.clit_glans_d.1.to_string()   + ","+ &self.clit_glans_d.2.to_string()   + ",\"" + &self.clit_glans_d.3 +"\"],\r\n"   +
                 " \"clit_glans_d_e\":[\""  + &self.clit_glans_d_e.0 +"\","  + &self.clit_glans_d_e.1.to_string() + ","+ &self.clit_glans_d_e.2.to_string() + ",\"" + &self.clit_glans_d_e.3 +"\"],\r\n" +
                 " \"clit_glans_l\":[\""    + &self.clit_glans_l.0 +"\","    + &self.clit_glans_l.1.to_string()   + ","+ &self.clit_glans_l.2.to_string()   + ",\"" + &self.clit_glans_l.3 +"\"],\r\n"   +
                 " \"clit_glans_l_e\":[\""  + &self.clit_glans_l_e.0 +"\","  + &self.clit_glans_l_e.1.to_string() + ","+ &self.clit_glans_l_e.2.to_string() + ",\"" + &self.clit_glans_l_e.3 +"\"],\r\n" +
                 " \"body_d\":[\""          + &self.body_d.0 +"\","          + &self.body_d.1.to_string()         + ",\""+ &self.body_d.2 +"\"],\r\n"       +
                 " \"body_d_e\":[\""        + &self.body_d_e.0 +"\","        + &self.body_d_e.1.to_string()       + ",\""+ &self.body_d_e.2 +"\"],\r\n"     +
                 " \"body_l\":[\""          + &self.body_l.0 +"\","          + &self.body_l.1.to_string()         + ",\""+ &self.body_l.2 +"\"],\r\n"       +
                 " \"body_l_e\":[\""        + &self.body_l_e.0 +"\","        + &self.body_l_e.1.to_string()       + ",\""+ &self.body_l_e.2 +"\"],\r\n"     +
                 " \"ball_r\":[\""          + &self.ball_r.0 +"\","          + &self.ball_r.1.to_string()         + ",\""+ &self.ball_r.2 +"\"],\r\n"       +
                 " \"ball_l\":[\""          + &self.ball_l.0 +"\","          + &self.ball_l.1.to_string()         + ",\""+ &self.ball_l.2 +"\"],\r\n"       +
                 " \"prepuce\":[\""         + &self.prepuce.0 +"\","         + &self.prepuce.1.to_string()        + ",\""+ &self.prepuce.2 +"\"],\r\n"      +
                 " \"invert\":[\""          + &self.invert.0 +"\","          + &self.invert.1.to_string()         + ",\""+ &self.invert.2 +"\"],\r\n"       +
                 " \"fork\":[\""            + &self.fork.0 +"\","            + &self.fork.1.to_string()           + ",\""+ &self.fork.2 +"\"],\r\n"       +
                 " \"perineum\":[\""        + &self.perineum.0 +"\","        + &self.perineum.1.to_string()       + ",\""+ &self.perineum.2 +"\"],\r\n"   +
                 " \"wrinkle\":[\""         + &self.wrinkle.0 +"\","         + &self.wrinkle.1.to_string()        + ",\""+ &self.wrinkle.2 +"\"],\r\n"      +
                 " \"areola\":[\""          + &self.areola.0 +"\","          + &self.areola.1.to_string()         + ","+ &self.areola.2.to_string()         + ",\"" + &self.areola.3 +"\"],\r\n"+
                 " \"nipple_d\":[\""        + &self.nipple_d.0 +"\","        + &self.nipple_d.1.to_string()       + ","+ &self.nipple_d.2.to_string()       + ",\"" + &self.nipple_d.3 +"\"],\r\n"+
                 " \"nipple_d_e\":[\""      + &self.nipple_d_e.0 +"\","      + &self.nipple_d_e.1.to_string()     + ","+ &self.nipple_d_e.2.to_string()     + ",\"" + &self.nipple_d_e.3 +"\"],\r\n"+
                 " \"nipple_h\":[\""        + &self.nipple_h.0 +"\","        + &self.nipple_h.1.to_string()       + ","+ &self.nipple_h.2.to_string()       + ",\"" + &self.nipple_h.3 +"\"],\r\n"+
                 " \"nipple_h_e\":[\""      + &self.nipple_h_e.0 +"\","      + &self.nipple_h_e.1.to_string()     + ","+ &self.nipple_h_e.2.to_string()     + ",\"" + &self.nipple_h_e.3 +"\"]\r\n}";
        response
    }
    pub fn load(data:String) -> Result<Erogenous> {
        let model: Value = serde_json::from_str(data.as_str())?;        
        let response = Erogenous {
            hood_c:         (json::string(&model["hood_c"][0]),         json::string(&model["hood_c"][1]),         json::string(&model["hood_c"][2])),
            hood_start:     (json::string(&model["hood_start"][0]),     json::u8(&model["hood_start"][1]),         json::string(&model["hood_start"][2])),
            hood_texture:   (json::string(&model["hood_texture"][0]),   json::u8(&model["hood_texture"][1]),       json::string(&model["hood_texture"][2])),
            hood_width:     (json::string(&model["hood_width"][0]),     json::u16(&model["hood_width"][1]),        json::string(&model["hood_width"][2])),
            hood_length:    (json::string(&model["hood_length"][0]),    json::u16(&model["hood_length"][1]),       json::string(&model["hood_length"][2])),
            lip_i_shape:    (json::string(&model["lip_i_shape"][0]),    json::string(&model["lip_i_shape"][1]),    json::string(&model["lip_i_shape"][2])),
            lip_i_texture:  (json::string(&model["lip_i_texture"][0]),  json::u8(&model["lip_i_texture"][1]),      json::string(&model["lip_i_texture"][2])),
            lip_i_width:    (json::string(&model["lip_i_width"][0]),    json::u8(&model["lip_i_width"][1]),        json::string(&model["lip_i_width"][2])),
            lip_i_length:   (json::string(&model["lip_i_length"][0]),   json::u16(&model["lip_i_length"][1]),      json::string(&model["lip_i_length"][2])),
            lip_i_length_r: (json::string(&model["lip_i_length_r"][0]), json::u16(&model["lip_i_length_r"][1]),    json::string(&model["lip_i_length_r"][2])),
            lip_i_length_l: (json::string(&model["lip_i_length_l"][0]), json::u16(&model["lip_i_length_l"][1]),    json::string(&model["lip_i_length_l"][2])),
            lip_o_shape:    (json::string(&model["lip_o_shape"][0]),    json::u8(&model["lip_o_shape"][1]),        json::string(&model["lip_o_shape"][2])),
            lip_o_texture:  (json::string(&model["lip_o_texture"][0]),  json::u8(&model["lip_o_texture"][1]),      json::string(&model["lip_o_texture"][2])),            
            clit_glans_d:   (json::string(&model["clit_glans_d"][0]),   json::u8(&model["clit_glans_d"][1]),       json::u16(&model["clit_glans_d"][2]),   json::string(&model["clit_glans_d"][3])),  
            clit_glans_d_e: (json::string(&model["clit_glans_d_e"][0]), json::u8(&model["clit_glans_d_e"][1]),     json::u16(&model["clit_glans_d_e"][2]), json::string(&model["clit_glans_d_e"][3])),
            clit_glans_l:   (json::string(&model["clit_glans_l"][0]),   json::u8(&model["clit_glans_l"][1]),       json::u16(&model["clit_glans_l"][2]),   json::string(&model["clit_glans_l"][3])),
            clit_glans_l_e: (json::string(&model["clit_glans_l_e"][0]), json::u8(&model["clit_glans_l_e"][1]),     json::u16(&model["clit_glans_l_e"][2]), json::string(&model["clit_glans_l_e"][3])),
            body_d:         (json::string(&model["body_d"][0]),         json::u16(&model["body_d"][1]),            json::string(&model["body_d"][2])),
            body_d_e:       (json::string(&model["body_d_e"][0]),       json::u16(&model["body_d_e"][1]),          json::string(&model["body_d_e"][2])),
            body_l:         (json::string(&model["body_l"][0]),         json::u16(&model["body_l"][1]),            json::string(&model["body_l"][2])),
            body_l_e:       (json::string(&model["body_l_e"][0]),       json::u16(&model["body_l_e"][1]),          json::string(&model["body_l_e"][2])),
            ball_r:         (json::string(&model["ball_r"][0]),         json::u8(&model["ball_r"][1]),             json::string(&model["ball_r"][2])),
            ball_l:         (json::string(&model["ball_l"][0]),         json::u8(&model["ball_l"][1]),             json::string(&model["ball_l"][2])),
            prepuce:        (json::string(&model["prepuce"][0]),        json::bool(&model["prepuce"][1]),          json::string(&model["prepuce"][2])),
            invert:         (json::string(&model["invert"][0]),         json::bool(&model["invert"][1]),           json::string(&model["invert"][2])),
            fork:           (json::string(&model["fork"][0]),           json::u8(&model["fork"][1]),               json::string(&model["fork"][2])),
            perineum:       (json::string(&model["perineum"][0]),       json::u8(&model["perineum"][1]),           json::string(&model["perineum"][2])),
            wrinkle:        (json::string(&model["wrinkle"][0]),        json::u8(&model["wrinkle"][1]),            json::string(&model["wrinkle"][2])),
            areola:         (json::string(&model["areola"][0]),         json::u16(&model["areola"][1]),            json::u16(&model["areola"][2]),    json::string(&model["areola"][3])),
            nipple_d:       (json::string(&model["nipple_d"][0]),       json::u8(&model["nipple_d"][1]),           json::u8(&model["nipple_d"][2]),   json::string(&model["nipple_d"][3])),
            nipple_d_e:     (json::string(&model["nipple_d_e"][0]),     json::u8(&model["nipple_d_e"][1]),         json::u8(&model["nipple_d_e"][2]), json::string(&model["nipple_d_e"][3])),
            nipple_h:       (json::string(&model["nipple_h"][0]),       json::u8(&model["nipple_h"][1]),           json::u8(&model["nipple_h"][2]),   json::string(&model["nipple_h"][3])),
            nipple_h_e:     (json::string(&model["nipple_h_e"][0]),     json::u8(&model["nipple_h_e"][1]),         json::u8(&model["nipple_h_e"][2]), json::string(&model["nipple_h_e"][3])),
        };
        Ok(response)
    }
}

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