<?php
error_reporting(E_ALL);
class node{
	public $fail  = null;
	public $child = array();
	public $index = -1;
	public $value = '';
	
	function __construct($value = ''){
		
		if(!empty($value))$this->value = $value;
		$this->fail = null;
		$this->child = array();
		$this->index = -1;
	}

	function test(){
	echo 1;
	}

}

$strs = array('he','she','her','sheet');

$root = new node();

function insert($root,$index,$str){
	$p = $root;
	$len = strlen($str);
	for($i=0;$i<$len;$i++){
		if(!isset($p->child[$str[$i]])){
			$p->child[$str[$i]] = new node($str[$i]);
		}
		$p = $p->child[$str[$i]];
		if($i == $len-1)$p->index= $index;
	}
}

function checkTrie($root){
	$qk = 0;
	$qv = array($root);
	while(isset($qv[$qk])){
		$now = $qv[$qk];
		foreach($now->child as $v){
			var_dump($v->value,$v->index,is_null($v->fail)?null:$v->fail->value);
			if(!empty($v->child)){
				$qv[] = $v;
			}
		}
		$qk++;
	}
}

function buildFailPoint($root){
	$qk = 0;
	$qv = array($root);
	while(isset($qv[$qk])){
		$now = $qv[$qk];
		foreach($now->child as $k=>$v){
			if(empty($now->value)){
				$v->fail = $root;
			}else{
				$p = $now->fail;
				while(!is_null($p)){
					if(isset($p->child[$k])){
						$v->fail = $p->child[$k];
						break;
					}
					$p = $p->fail;
				}
				if(is_null($p))$v->fail = $root;
			}
			if(!empty($v->child)){
				$qv[] = $v;
			}
		}
		$qk++;
	}
}

function search($str,$root){
	$res = array();
	$len = strlen($str);
	$p = $root;
	for($i = 0;$i<$len;$i++){
		$s = $str[$i];
		while(!isset($p->child[$s]) && !empty($p->value)){
			$p = $p->fail;
		}
		if(!isset($p->child[$s]))continue;
		$p = $p->child[$s];
		$t = $p;
		while(!empty($t->value)){
			if($t->index > -1){
				$res[] = array($i,$t->index);
			}
			$t = $t->fail;
		}
	}
	return $res;
}

foreach($strs as $k=>$v){
	insert($root,$k,$v);
}

//checkTrie($root);
buildFailPoint($root);
//checkTrie($root);
$searcher = 'fdshkjhgfdahchesheher';
$res = search($searcher,$root);
var_dump($searcher);
foreach($res as $v){
	var_dump($v[0],$strs[$v[1]]);
}

?>
