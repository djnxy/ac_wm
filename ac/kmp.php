<?php
$s = 'fdsjhouijihpofdjkfdsjkdihgfdjsjhoigihgfiohigjoihjfdjkfdsjkfsjhiohjphgkfhjkgj';
$p = 'fdjkfdsjk';
$next = array();

function getNext($str){
	$size = strlen($str);
	$next = array(-1);
	$i = 0;
	$j = -1;
	while($i < $size){
		if($j == -1 || $str[$i] == $str[$j]){
			$i++;
			$j++;
			$next[$i] = $str[$i] != $str[$p]?$j:$next[$j];
		}else{
			$j = $next[$j];
		}
	}
	return $next;
}

function search($str,$text){
	$next = getNext($str);
	$size = strlen($str);
	$text_size = strlen($text);
	$i = 0;
	$j = 0;
	$res = array();
	while($i < $text_size && $j<$size){
		if($j == -1 || $text[$i] == $str[$j]){
			$i++;
			$j++;
			if($j == $size){
				$res[] = $i - $j;
				$j = $next[$j];
			}
		}else{
			$j = $next[$j];
		}
	}
	return $res;
}

$res = search($p,$s);
$size = strlen($p);
var_dump($s);
foreach($res as $v){
var_dump($v,substr($s,$v,$size));
}
?>
