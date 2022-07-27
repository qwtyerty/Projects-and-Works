using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Pipeline : MonoBehaviour
{
    //Store the location of the enterance and exit of a transport
    public GameObject enter;
    public GameObject exit;

    //Self Explanatorty
    public float pipeSpeed;

    //Array of limits to apply to ions that pass through
    public float[] exitClamp;

    //Whether a pipe is active or not
    public bool activeState = true;

    //Pop-up to let the player know that the ion is incorect
    public GameObject incPopUp;

    // Stores materials in order to show an order of Ions for reference in the editor
    public Material[] matArr;

    // Array of booleans to store which ions are allowed to enter
    public bool[] canEnter;
    private int ionType;
                           // H2O      Cl-    K+    K+(Type 2)    Na+
    public int[] exitState = { 0,       0,    0,       0,          0 };

    private GameObject obj;
    private float[] tempBound = {-100f, 100f, -1f, 5f, -100f, 100f};
    

    private void OnTriggerEnter(Collider collision)
    {
        //Store the ion in an object and check its type
        obj = collision.gameObject;
        if (obj.GetComponent<IonBehavior>() != null)
            ionType = obj.GetComponent<IonBehavior>().ionType;

        // Activate pop-up for invalid ions
        if(ionType != null)
            if (!canEnter[ionType])
                incPopUp.SetActive(true);

        // Lets the ion in 
        else if (activeState && !collision.gameObject.GetComponent<PickupBehavior>().isGrabbed)
        {
            incPopUp.SetActive(false);
            //Disable Gravity on hitting the pipe
            collision.gameObject.GetComponent<Rigidbody>().useGravity = false;

            //Give a temp bound until exit
            if (obj.GetComponent<LogicalBounding>() != null)
            {
                obj.GetComponent<LogicalBounding>().bounds = tempBound;
            }
            // Tell ions of the same type to move towards the pipe and dea
            if(obj.GetComponent<IonSignalBehavior>() != null)
                obj.GetComponent<IonSignalBehavior>().SignalCoIons(obj.transform.position, this.gameObject);
        }
    }

    // Moving the ion through the pipe
    private void OnTriggerStay(Collider collision)
    {
        obj = collision.gameObject;
        var i = obj.GetComponent<IonBehavior>().ionType;
        // Sanity Check
        if (!canEnter[i])
            incPopUp.SetActive(true);

        //Controlls movement
        else if (activeState && !collision.gameObject.GetComponent<PickupBehavior>().isGrabbed)
        {
            incPopUp.SetActive(false);
            //Get the direction with a normalized vector
            var exitVector = (enter.transform.position - exit.transform.position).normalized;

            //Apply it
            collision.gameObject.GetComponent<Rigidbody>().velocity = pipeSpeed * -exitVector;

            //End pathfinding into the pipe
            if (!obj.GetComponent<TravelLogic>() != null)
                if (!obj.GetComponent<TravelLogic>().isComplete)
                {
                    obj.GetComponent<TravelLogic>().isComplete = true;
                    obj.GetComponent<TravelLogic>().isActive = false;
                }
        }
    }

    private void OnTriggerExit(Collider collision)
    {
        obj = collision.gameObject;
        var i = obj.GetComponent<IonBehavior>().ionType;

        // Sanity Check
        if (!canEnter[i])
            incPopUp.SetActive(true);

        // Set gravity flags back to normal and give the ion an exit velocity
        else if (activeState && !collision.gameObject.GetComponent<PickupBehavior>().isGrabbed)
        {
            incPopUp.SetActive(false);
            //Get the direction with a normalized vector
            var exitVector = (enter.transform.position - exit.transform.position).normalized;

            //Re-enable Gravity then give an exit velocity to prevent re-entry
            obj.GetComponent<Rigidbody>().useGravity = true;
            obj.GetComponent<Rigidbody>().velocity = pipeSpeed * -exitVector;
            obj.GetComponent<Rigidbody>().velocity += new Vector3(0f, 0f, 0.5f);

            //Get assign its new clamp
            if (obj.GetComponent<LogicalBounding>() != null)
            {
                obj.GetComponent<LogicalBounding>().bounds = exitClamp;
            }

            // Increment the appropriate exitState entry
            exitState[i]++;
        }
    }
}
