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
				var_dump($v->value,$v->index);
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


//$ac = new ac($strs);
//$ac->checkTrie();
//$res = $ac->search('fdshkjhgfdahchesheher');
//var_dump($res);
//exit;


$keysets = array();
$multikey = array();
$patterns = array();

function setTVKey(){
	global $patterns,$keysets,$multikey;
	$ps = array();
	$f = fopen('/data1/dev/wm/tvKey.txt','r');
	while(!feof($f)){
		$line = trim(fgets($f));
		if($line==''){continue;}
		$line = explode("\t",$line);
		$keystr = trim($line[1]);
		if($line[0]=='' || $keystr=='')continue;
		$words = explode(',',$keystr);
		foreach($words as $v){
			$word = trim($v);
			if($word=='')continue;
			$multi = explode('|',$word);
			if(count($multi)>1){
				foreach($multi as $single){
					if(!isset($multikey[$single])){
						$patterns[]=$single;
					}
					if(empty($multikey[$single]) || !in_array($word,$multikey[$single])){
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
		$keys = array_values($keys);
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

setTVKey();

$ac = new ac($patterns);

$t1 = microtime(true);
$fdata = fopen('/data1/dev/wm/in/13_zf_safe.txt','r');
while(!feof($fdata)){
	$linestr = trim(fgets($fdata));
	if($linestr==''){continue;}
	$line = explode("\t",$linestr);
	$str = $line[19].$line[23];
	$keys = $ac->search($str);
	if(!empty($keys)){
		$oids = getTV($keys);
		if(!empty($oids)){
			foreach($oids as $v){
				error_log($line[2]."\t".$line[0]."\t".$v."\n",3,'/data1/dev/wm/out/zf_mention.log');
			}
		}
	}
}
fclose($fdata);
var_dump(microtime(true)-$t1);exit;
?>
