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

function setKey(){
	global $patterns,$keysets,$multikey;
	$f = fopen('/data1/dev/wm/sensitive_words','r');
	while(!feof($f)){
		$line = trim(fgets($f));
		if(empty($line)){continue;}
		$multi = explode('|',$line);
		if(count($multi)>1){
			foreach($multi as $single){
				if(!isset($multikey[$single])){
					$patterns[]=$single;
				}
				if(!in_array($line,$multikey[$single])){
					$multikey[$single][] = $line;
				}
			}
			!isset($keysets[$line]) && $keysets[$line]='';
		}else{
			if(!isset($keysets[$multi[0]])){
				$patterns[] = $multi[0];
			}
			!isset($keysets[$multi[0]]) && $keysets[$multi[0]]='';
		}
	}
	$patterns = array_unique($patterns);
	fclose($f);
}

function issafe($keys){
	if(empty($keys)){
		return true;
	}
	global $keysets,$multikey;
	$size = count($keys);
	if($size==1){
		if(isset($keysets[$keys[0]])){
			return false;
		}
	}else{
		$multis = array();
		foreach($keys as $k=>$key){
			if(isset($keysets[$key])){
				return false;
			}
			if(isset($multikey[$key])){
				$multis = array_merge($multis,$multikey[$key]);
			}
		}
		$multis = array_unique($multis);
		$keys[] = '';
		foreach($multis as $v){
			$childs = explode('|',$v);
			if(count(array_intersect($childs,$keys)) == count($childs)){
				return false;
			}
		}
	}
	return true;
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

setKey();

init();

$t1 = microtime(true);
$fdata = fopen('/data1/dev/wm/online/13_yc.txt','r');
while(!feof($fdata)){
	$linestr = trim(fgets($fdata));
	if(empty($linestr)){continue;}
	$line = explode("\t",$linestr);
	$keys = search($line[3]);
	if(!empty($keys)){
		if(issafe($keys)){
			error_log($linestr."\n",3,'/data1/dev/wm/in/13_yc_safe.txt');
		}
	}else{
		error_log($linestr."\n",3,'/data1/dev/wm/in/13_yc_safe.txt');
	}
}
fclose($fdata);
var_dump(microtime(true)-$t1);exit;
?>
