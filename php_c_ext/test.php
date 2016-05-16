<?php
if(!extension_loaded('nxytest')) {
	dl('nxytest.' . PHP_SHLIB_SUFFIX);
}
//var_dump(nxytest());

//$keys = array('he','she','her','shr','say');
$keys = array('我们','我','们图','我是');

$testobj = new testobj($keys);
//$testobj = new testobj(array(1,2,3,4,5,6));
//var_dump($testobj->memory);
//$testobj->learn("love");
//var_dump($testobj->memory);
//var_dump($testobj->keys);
//var_dump($testobj->check_keys());

//$res = $testobj->search('fdsherfdssayfds');
$res = $testobj->search('我们图什么，我是不图什么');
var_dump($res);
//$time = microtime(true);
//for($i=0;$i<10000;$i++){
//	//$res = $testobj->search('fdsherfdssayfds');
//	$res = $testobj->search('我们图什么，我是不图什么');
//}
//var_dump(microtime(true)-$time);
?>
