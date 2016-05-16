<?php
$str = 'abcdefgjksjihnblkdjsgfdjk';
$keysets = array();
$multikey = array();
$patterns = array();
//$patterns = array('abcdefg','abfdsg','gfhdsof','ophgofj','sg');
$m = 2;
$b = 2;
$shift = array();
$hash = array();
$prefix = array();

function setTVKey(){
	global $patterns,$keysets,$multikey;
	$ps = array();
	$f = fopen('/data1/dev/wm/tvKey.txt','r');
	while(!feof($f)){
		$line = trim(fgets($f));
		if(empty($line)){continue;}
		$line = explode("\t",$line);
		$keystr = trim($line[1]);
		if(empty($line[0]) || empty($keystr))continue;
		$words = explode(',',$keystr);
		foreach($words as $v){
			$word = trim($v);
			if(empty($word))continue;
			$multi = explode('|',$word);
			if(count($multi)>1){
				foreach($multi as $single){
					if(!isset($multikey[$single])){
						$patterns[]=$single;
					}
					if(!in_array($word,$multikey[$single])){
						$multikey[$single][] = $word;
					}
				}
				$keysets[$word][] = $line[0];
			}else{
				if(!isset($keysets[$multi[0]])){
					$patterns[] = $multi[0];
				}
				$keysets[$multi[0]][] = $line[0];
			}
		}
	}
	fclose($f);
}

function getTV($keys){
	$tvs = array();
	if(empty($keys)){
		return $tvs;
	}
	global $keysets,$multikey;
	$size = count($keys);
	if($size==1){
		if(isset($keysets[$keys[0]])){
			foreach($keysets[$keys[0]] as $oid){
				$tvs[] = $oid;	
			}
		}
	}else{
		$combined = array();
		foreach($keys as $k=>$key){
			for($i=0;$i<$size;$i++){
				if($i == $k){
					continue;
				}
				$combined[] = $key.'|'.$keys[$i];
			}
			if(isset($keysets[$key])){
				foreach($keysets[$key] as $oid){
					$tvs[] = $oid;	
				}
			}
		}
		foreach($combined as $v){
			if(isset($keysets[$v])){
				foreach($keysets[$v] as $oid){
					$tvs[] = $oid;	
				}
			}
		}
	}
	return array_unique($tvs);
}

function init(){
	global $patterns,$m,$b,$shift,$hash,$prefix;
	foreach($patterns as $pat){
		$index = 0;
		$size = strlen($pat);
		while($index < $m-1 && $size > $index + $b - 1){
			$block = substr($pat,$index,$b);
			$dis = $m-$index-$b;
			if(!isset($shift[$block]) || $dis < $shift[$block]){
				$shift[$block] = $dis;
			}
			if($dis == 0){
				$hash[$block][] = $pat;
				$pre = substr($pat,0,$b);
				if(!isset($prefix[$block]) || !isset($prefix[$block][$pre])){
					$prefix[$block][$pre] = 1;
				}
			}
			$index++;
		}
	}
}

function search($str){
	global $patterns,$m,$b,$shift,$hash,$prefix;
	$size = strlen($str);
	$index = $m-$b;
	if($size<$b){
		return;
	}
	$res = array();
	while($index+$b < $size+1){
		$block = substr($str,$index,$b);
		if(isset($shift[$block])){
			if($shift[$block] == 0){
				$pre = substr($str,$index+$b-$m,$b);
				if(isset($prefix[$block][$pre])){
					foreach($hash[$block] as $pat){
						if(substr($pat,0,$b) == $pre){
							$stop_token = false;
							for($i = 0;$i < strlen($pat);$i++){
								if(substr($pat,$i,1) != substr($str,$index+$b-$m+$i,1)){
									$stop_token = true;
									$break;
								}
							}
							if(!$stop_token){
								$res[] = $pat;
							}
						}
					}
				}
				$index+=1;
			}else{
				$index+=$shift[$block];
			}
		}else{
			$index+=$m-$b+1;
		}
	}
	return array_unique($res);
}

setTVKey();

init();

$t1 = microtime(true);
$fdata = fopen('/data1/dev/wm/in/13_yc_safe.txt','r');
while(!feof($fdata)){
	$linestr = trim(fgets($fdata));
	if(empty($linestr)){continue;}
	$line = explode("\t",$linestr);
	$keys = search($line[3]);
	if(!empty($keys)){
		$oids = getTV($keys);
		if(!empty($oids)){
			foreach($oids as $v){
				error_log($line[2]."\t".$line[0]."\t".$v."\n",3,'/data1/dev/wm/out/yc_mention.log');
			}
		}
	}
}
fclose($fdata);
var_dump(microtime(true)-$t1);exit;
?>
