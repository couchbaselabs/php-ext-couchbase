<?php
/**
 * This example demonstrates how to update individual fields in a document
 * with implicit concurrency handling
 */
class CouchbaseUpdate extends Couchbase {

    private $max_retries = 1;

    /**
     * recurses through the path to find the closest parent
     * object of the field to be updated
     */
    private function _recursive_update($obj, $path, $update, $depth)
    {
        $node = $path[$depth];
        $next_node = $path[$depth+1];

        if(!$obj){
            throw new Exception("Field not found");
        }
        if(!$path || gettype($path) != 'array'){
            throw new Exception("Invalid path");
        }

        /* update at the root level */
        if(count($path) == 1){
            reset($update);
            $k = key($update);
            $v = current($update);

            if(isset($obj[$k])){
                return (object) array_replace_recursive((array) $obj, (array) $update);
            }
            else{
                return (object) array_merge_recursive((array) $obj, (array) $update);
            }
        }

        /**
         * special handling for updating arrays at depth 2
         * as arrays have to be replaced
         */
        if(count($path) == 2){
            if(gettype($obj[$path[1]]) == 'array'){
                $val = $obj[$path[1]];
                $new_val = array_merge_recursive($val, $update);
                $obj[$path[1]] = $new_val;
                return $obj;
            }
        }

        /**
         * similar to special handling above for arrays but at
         * a varied depth
         */
        if($depth == count($path) - 2){
            $temp = (array)$obj[$node];
            if(gettype($temp[$next_node]) == 'array'){
                reset($update);
                $k = key($update);
                $v = current($update);
                $val = $temp[$next_node];

                if(isset($val[$k])){
                    $new_val = array_replace_recursive($val, $update);
                }
                else {
                    $new_val = array_merge_recursive($val, $update);
                }

                if(gettype($obj[$node]) == 'object'){
                    $obj[$node]->$next_node = $new_val;
                }
                else{
                    $obj[$node][$next_node] = $new_val;
                }
                return;
            }
        }

        /* handling scalar values and JSON Object */
        if($depth == count($path) - 1){
            reset($update);
            $k = key($update);
            $v = current($update);

            if(gettype($obj[$node]) == 'object'){
                $obj[$node]->$k = $v;
            }
            else{
                $obj[$node][$k] = $v;
            }
            return;
        }

        if($depth == 0){
            /* ignore "root" */
            self::_recursive_update((array)$obj[$next_node], $path, $update, $depth + 2);
        }

        else{
            self::_recursive_update((array)$obj[$node], $path, $update, $depth + 1);
        }
        return $obj;
    }

    /* merge updates individually */
    private function _merge($doc, $updates){
        foreach($updates as $k => $v){
            $doc = self::_recursive_update((array)$doc, $v['path'], array($k => $v['val']), 0);
        }
        return $doc;
    }

    /**
     * Partial Update: Update/Add individual fields in a JSON Document safely
     * Complete Update: Create/Replace the entire Document,
     *                      the given object is encoded to JSON
     * @param $key Document Identifier
     * @param $obj Either Entire Document object for complete update OR
     *      An array with partial updates as keys where
     *          key is an index for an array otherwise the field name.
     *      Each key is associated with path and val properties.
     *      Path is an array of the field path from the root (doc's parent object) and
     *          Val is the new value of the field.
     * @param $is_partial Boolean to identify a partial update
     * @params $cas, $persist_to, $replicate_to function similar to SET operation
     * @return Boolean true on SUCCESS else false
     */

    public function update($key, $obj, $is_partial = false, $expiry = 0, $persist_to = 0, $replicate_to = 0) {
        $counter = 1;
        $n_json = NULL;
        do{
            $v_json = parent::get($key, NULL, $cas);
            if($obj){
                if($is_partial == true){
                    if(!$cas){
                        throw new Exception("No existing document for partial update");
                    }
                    $valGet = json_decode($v_json);
                    $val = self::_merge($valGet, $obj);
                    $n_json = json_encode($val);
                }
                else{
                    $cas = 0;
                    $n_json = json_encode($obj);
                }
            }
            $rv = parent::set($key, $n_json, $expiry , $cas, $persist_to, $replicate_to);

        }while($rv == false && ++$counter <= $this->max_retries);
        return $rv;
    }

    /**
     * Set the max retries count
     * @param max_retries Positive number
     */
    public function setMaxRetries($max_retries){

        if(!$max_retries || is_nan($max_retries) || $max_retries <= 0){
            throw new Exception("Illegal count for retries");
        }

        $this->max_retries = $max_retries;
    }
}
?>
