using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;


//Set up: put on an object you want to be a pickup, and have a disination for the object
// Label sections can be removed

/** WARNING **/
    //If a VR input system/loader is being used this will not work, disable vr/xr before trying to bugfix
    //issues
public class PickupBehavior : MonoBehaviour
{
    public Transform grabDest;
    public bool isGrabbed = false;

    private int rotAxis = 0;
    private GameObject label;
    private Vector3 scale;
    public Quaternion startRotation;//The initial global rotation of the object at the start of the room.
    private Transform parent;

    // Start is called before the first frame update
    void Start()
    {
        scale = this.gameObject.transform.lossyScale;
        startRotation = this.gameObject.transform.rotation;
        if (this.gameObject.transform.Find("Label"))
        {
            label = this.gameObject.transform.Find("Label").gameObject;
        }
        else
        {
            Debug.Log("Label is missing from " + this.gameObject.name);
        }
    }

    //Grabbing
    private void OnMouseDown()
    {
        Debug.Log("Mouse Down Called on " + this.gameObject.name);
        if (this.gameObject.transform.parent != null)
            this.gameObject.transform.parent = null;

        foreach (Transform child in this.gameObject.transform)
            if (child.gameObject.GetComponent<Rigidbody>())
                if (child.gameObject.GetComponent<Rigidbody>().useGravity == true)
                    child.gameObject.GetComponent<Rigidbody>().useGravity = false;

        GetComponent<Rigidbody>().useGravity = false;
        GetComponent<Rigidbody>().velocity = Vector3.zero;
        GetComponent<Rigidbody>().angularVelocity = Vector3.zero;
        this.transform.position = grabDest.position;
        this.transform.parent = GameObject.Find("Destination").transform;

        isGrabbed = true;
    }

    //Releasing
    void OnMouseUp()
    {
        this.transform.parent = null;

        foreach (Transform child in this.gameObject.transform)
        {
            if (child.gameObject.GetComponent<Rigidbody>())
            {
                if (child.gameObject.GetComponent<Rigidbody>().useGravity == false)
                {
                    child.gameObject.GetComponent<Rigidbody>().useGravity = true;
                }
            }
        }
        GetComponent<Rigidbody>().useGravity = true;
        isGrabbed = false;
    }

    void Update()
    {
        if (isGrabbed)
        {
            
            this.transform.position = grabDest.position;
            rotate(rotAxis);

            //Reset rotation
            if (Input.GetKey(KeyCode.Space))
            {
                transform.eulerAngles = new Vector3(0.0f, transform.eulerAngles.y, 0.0f);
                this.GetComponent<Rigidbody>().velocity = Vector3.zero;//Resets the velocity of the currently grabbed object so that is will not move in a direction once released.

                this.GetComponent<Rigidbody>().angularVelocity = Vector3.zero;//Resets the angular velocity of the currently grabbed object such that it will no longer rotate once it is released
                                                                              //Or after the space bar is released.
            }

            //Switch rotation axis
            if (Input.GetKeyUp(KeyCode.Q))
                rotAxis = (rotAxis + 1) % 3;

            //Push foward using scroll wheel
            if (Input.mouseScrollDelta.y > 0)
            {
                grabDest.position += Camera.main.transform.forward * Time.deltaTime * 20.0f;//Moves the destination game object towards the player.
            }

            //Bring Closer
            else if (Input.mouseScrollDelta.y < 0)
            {
                grabDest.position += Camera.main.transform.forward * Time.deltaTime * -20.0f;//Moves the destination game object away from the player.
            }

            if (scale != this.gameObject.transform.lossyScale)
                adjustScale(scale);
        }

    }

    //Rotate on axis with e,r switch axes with q
    private void rotate(int axis)
    {
        if (Input.GetKey(KeyCode.E))
        {
            switch (axis)
            {
                case 0:
                    this.transform.Rotate(0f, 0.3f, 0f);
                    break;
                case 1:
                    this.transform.Rotate(0.0f, 0.0f, -0.3f);
                    break;
                case 2:
                    this.transform.Rotate(0.3f, 0.0f, 0.0f);
                    break;
            }
        }
        else if (Input.GetKey(KeyCode.R))
        {
            switch (axis)
            {
                case 0:
                    this.transform.Rotate(0f, -0.3f, 0f);
                    break;
                case 1:
                    this.transform.Rotate(0.0f, 0.0f, 0.3f);
                    break;
                case 2:
                    this.transform.Rotate(-0.3f, 0.0f, 0.0f);
                    break;
            }
        }

    }

    //Reset scale if it breaks
    private void adjustScale(Vector3 targetScale)
    {
        /***
            Adjusts the lossy scale of an object by removing the parent if needed then changing the local scale
        ***/
        if (this.transform.parent == null)
        {
            this.transform.localScale = targetScale;
        }

        else
        {
            parent = this.transform.parent;

            this.transform.parent = null;
            this.transform.localScale = targetScale;
            this.transform.parent = parent;
        }
    }
}
