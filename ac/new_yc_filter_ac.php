<?php
error_reporting(E_ALL);

class node{
	public $fail  = null;
	public $child = array();
	public $index = -1;
	public $value = '';
	
	function __construct($value = ''){
		
		$this->value = $value;
		$this->fail = null;
		$this->child = array();
		$this->index = -1;
	}
}

class ac{
	public $root = null;
	public $keys = array();

	function __construct($keys = array()){
		$this->root = new node();
		$this->keys = $keys;
		foreach($keys as $k=>$v){
			$this->insert($k,$v);
		}
		$this->buildFailPoint();
	}

	function insert($index,$str){
		$p = $this->root;
		$len = mb_strlen($str,'utf8');
		for($i=0;$i<$len;$i++){
			$s = mb_substr($str,$i,1,'utf8');
			if(!isset($p->child[$s])){
				$p->child[$s] = new node($s);
			}
			$p = $p->child[$s];
			if($i == $len-1)$p->index= $index;
		}
	}

	function checkTrie(){
		$qk = 0;
		$qv = array($this->root);
		while(isset($qv[$qk])){
			$now = $qv[$qk];
			foreach($now->child as $v){
				var_dump($now->value,$v->fail->value,$v->value,$v->index);
				if(!empty($v->child)){
					$qv[] = $v;
				}
			}
			$qk++;
		}
	}

	function buildFailPoint(){
		$qk = 0;
		$qv = array($this->root);
		while(isset($qv[$qk])){
			$now = $qv[$qk];
			foreach($now->child as $k=>$v){
				if($now->value == ''){
					$v->fail = $this->root;
				}else{
					$p = $now->fail;
					while(!is_null($p)){
						if(isset($p->child[$k])){
							$v->fail = $p->child[$k];
							break;
						}
						$p = $p->fail;
					}
					if(is_null($p))$v->fail = $this->root;
				}
				if(!empty($v->child)){
					$qv[] = $v;
				}
			}
			$qk++;
		}
	}

	function search($str){
		$res = array();
		$len = mb_strlen($str,'utf8');
		$p = $this->root;
		for($i = 0;$i<$len;$i++){
			$s = mb_substr($str,$i,1,'utf8');
			while(!isset($p->child[$s]) && $p->value != ''){
				$p = $p->fail;
			}
			if(!isset($p->child[$s]))continue;
			$p = $p->child[$s];
			$t = $p;
			while($t->value != ''){
				if($t->index > -1){
					$res[] = array($i,$t->index);
				}
				$t = $t->fail;
			}
		}
		return $this->getKey($res);
	}

	function getKey($res){
		$out = array();
		foreach($res as $v){
			$out[] = $this->keys[$v[1]];
		}
		return array_unique($out);
	}

}

//$strs = array('he','she','her','shr','say');
//$ac = new ac($strs);
//$ac->checkTrie();
//$time = microtime(true);
//for($i=0;$i<10000;$i++){
//	$res = $ac->search('fdsherfdssayfds');
//}
//$res = $ac->search('fdsherfdssayfds');
//var_dump($res);
//var_dump(microtime(true)-$time);
//exit;




$keysets = array();
$multikey = array();
$patterns = array();

function setKey(){
	global $patterns,$keysets,$multikey;
	$f = fopen('/data1/dev/wm/sensitive_words','r');
	while(!feof($f)){
		$line = trim(fgets($f));
		if($line == ''){continue;}
		$multi = explode('|',$line);
		if(count($multi)>1){
			foreach($multi as $single){
				if(!isset($multikey[$single])){
					$patterns[]=$single;
				}
				if(empty($multikey[$single]) || !in_array($line,$multikey[$single])){
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

function restore($str){
	error_log($str."\n",3,'/data1/dev/wm/in/13_yc_safe.txt');
}

setKey();

if(!extension_loaded('nxytest')) {
        dl('nxytest.' . PHP_SHLIB_SUFFIX);
}
$ac = new testobj($patterns);
//var_dump($ac->check_keys());
//exit;

$t1 = microtime(true);
$fdata = fopen('/data1/dev/wm/online/13_yc.txt','r');
while(!feof($fdata)){
	$linestr = trim(fgets($fdata));
	if($linestr == ''){continue;}
	$line = explode("\t",$linestr);
	if($line[0] == '3964510413023608'){
		var_dump($line[3],$ac->search($line[3]));
	}else{
		continue;
	}
	$keys = $ac->search($line[3]);
	if(!empty($keys)){
		if(issafe($keys)){
			restore($linestr);
		}
	}else{
			restore($linestr);
	}
}
fclose($fdata);
var_dump(microtime(true)-$t1);exit;
?>
